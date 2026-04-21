/**
 * @file display.c
 * @brief DISLPAY control and PRINT SCREEN implementations.
 */
#include <stdint.h>
#include <stdbool.h>
#include "bsp.h"
#include "bsp_pinout.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h" 
#include "freertos/task.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_ili9341.h"

/** @brief Mutex to protect multithreaded access to brightness variables */
static SemaphoreHandle_t brightness_mutex = NULL;

/** @brief Current brightness level of the backlight (0-255) */
static uint8_t current_brightness = 0;

/** @brief Target brightness level of the backlight to transition to (0-255) */
static uint8_t target_brightness  = 255;

/** @brief Amount to change the brightness per step */
#define RAMP_STEP  2

/** @brief Interval between brightness changes in milliseconds */
#define RAMP_MS    20

/** @brief Hight pixel display value */
#define LCD_HI_RES 320
/** @brief Lenght pixel display value */
#define LCD_VE_RES 240

/** @brief Macro to find the minimum of two values */
#define MIN(a, b) ((a) < (b) ? (a) : (b))

/** @brief Macro to find the maximum of two values */
#define MAX(a, b) ((a) > (b) ? (a) : (b)) 

#define RGB888_TO_565(r, g, b)  (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))

esp_lcd_panel_handle_t panel_handle = NULL;
esp_lcd_panel_io_handle_t io_handle = NULL;
/** @brief Buffer for the complete frame */
static uint16_t *frame_buffer = NULL;
/** @brief Semaphore to synchronize DMA transfers */
static SemaphoreHandle_t dma_done = NULL;


/**
 * @brief Steps a current value closer to a target value by at most a specified step size.
 * 
 * Takes care of both increasing and decreasing the value, while ensuring it never 
 * overshoots the target.
 * 
 * @param current The current value.
 * @param target The requested target value.
 * @param step The maximum change allowed in this step.
 * @return uint8_t The new value, closer to or equal to the target.
 */
static uint8_t step_toward(uint8_t current, uint8_t target, uint8_t step); 

/**
 * @brief Callback invoked when a color transfer is completed.
 * 
 * @param panel_io Panel I/O handle.
 * @param edata Pointer to the event data.
 * @param user_ctx User-defined context data.
 * @return bool True if a higher priority task was woken, false otherwise.
 */
static bool on_color_trans_done(esp_lcd_panel_io_handle_t panel_io,
                                 esp_lcd_panel_io_event_data_t *edata,
                                 void *user_ctx) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(dma_done, &xHigherPriorityTaskWoken);
    return xHigherPriorityTaskWoken == pdTRUE;
}
/**
 * @brief Initializes the display and configure it for its use.
 * 
 * Configures the screen to it use.
 * 
 *
 * @return esp_err_t ESP_OK on success, or ESP_ERR_INVALID_ARG on failure.
 */
esp_err_t init_display(void)
{
    if (gpio_reset_pin(BSP_TFT_BL) == ESP_ERR_INVALID_ARG) return ESP_ERR_INVALID_ARG;
    if (gpio_set_direction(BSP_TFT_BL, GPIO_MODE_OUTPUT) == ESP_ERR_INVALID_ARG) return ESP_ERR_INVALID_ARG;

    ledc_timer_config_t timer = {    
        .speed_mode      = LEDC_LOW_SPEED_MODE,
        .timer_num       = BSP_BL_LEDC_TIMER,
        .duty_resolution = BSP_BL_LEDC_RES,
        .freq_hz         = BSP_BL_LEDC_FREQ,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer));
    vTaskDelay(pdMS_TO_TICKS(10));

    ledc_channel_config_t ch = {
        .gpio_num            = BSP_TFT_BL,
        .speed_mode          = LEDC_LOW_SPEED_MODE,
        .channel             = BSP_BL_LEDC_CHANNEL,
        .timer_sel           = BSP_BL_LEDC_TIMER,
        .duty                = 255,
        .hpoint              = 0,
        .flags.output_invert = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ch)); 

    brightness_mutex = xSemaphoreCreateMutex(); 
    dma_done = xSemaphoreCreateBinary();  // 1. Create binary semaphore before io_config

    // 2. Declare io_config BEFORE using it, including the callback
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num         = BSP_TFT_DC,
        .cs_gpio_num         = BSP_TFT_CS,
        .pclk_hz = 20 * 1000 * 1000,
        .lcd_cmd_bits        = 8,
        .lcd_param_bits      = 8,
        .spi_mode            = 0,
        .trans_queue_depth   = 1,
        .on_color_trans_done = on_color_trans_done,  // Callback to signal DMA finish
        .user_ctx            = NULL,
    };
    esp_lcd_new_panel_io_spi(SPI2_HOST, &io_config, &io_handle);  // 3. One single call to initialize

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = BSP_TFT_RST,
        .rgb_ele_order  = LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel = 16,
    };
    esp_lcd_new_panel_ili9341(io_handle, &panel_config, &panel_handle);

    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);
    esp_lcd_panel_invert_color(panel_handle, false);
    esp_lcd_panel_swap_xy(panel_handle, true);
    esp_lcd_panel_mirror(panel_handle, true, false);
    esp_lcd_panel_disp_on_off(panel_handle, true);

    // 4. Allocate frame_buffer in DMA
    frame_buffer = heap_caps_malloc(LCD_HI_RES * LCD_VE_RES * sizeof(uint16_t), MALLOC_CAP_DMA);
    if (frame_buffer == NULL) return ESP_ERR_NO_MEM;

    xTaskCreate(backlight_task, "backlight", 4096, NULL, 1, NULL);
    bsp_backlight_set_target(120);
    xTaskCreate(display_effect_task, "display_effect", 8192, NULL, 1, NULL);
    return ESP_OK;                   
}

/**
 * @brief Sets the duty cycle for the backlight screen via LEDC.
 * 
 * @param backlight backlight duty cycle value (0-255).
 */
void bsp_backlight_set(uint8_t backlight) 
{
    // Schedule next-cycle duty modification inside ESP subsystem
    ledc_set_duty(LEDC_LOW_SPEED_MODE, BSP_BL_LEDC_CHANNEL, backlight);
    // Commit schedule synchronously pushing to hardware pins immediately
    ledc_update_duty(LEDC_LOW_SPEED_MODE, BSP_BL_LEDC_CHANNEL);
}

/**
 * @brief Sets the target brightness for the backlight.
 * 
 * The backlight_task will smoothly transition to this target brightness.
 * 
 * @param brightness Target brightness level (0-255).
 */
void bsp_backlight_set_target(uint8_t brightness)
{
    // Safely reserve exclusivity rights or wait slightly up to 10ms
    if (xSemaphoreTake(brightness_mutex, pdMS_TO_TICKS(10)) == pdTRUE) 
    { 
        // Once reserved, overwrite current globally tracked target without conflicts
        target_brightness = brightness; 
        // Yield explicit reservation right back to threadpool
        xSemaphoreGive(brightness_mutex);
    }
}

/**
 * @brief Thread-safe getter for the target brightness.
 * 
 * @return uint8_t Current target brightness level.
 */
static uint8_t get_target_brightness(void) 
{
    // Ready localized variable ensuring fallback response initialized to 255
    uint8_t val = 255;
    // Seek brief lock guaranteeing thread safety read integrity
    if (xSemaphoreTake(brightness_mutex, pdMS_TO_TICKS(10)) == pdTRUE) { 
        // Securely snapshot current value directly into isolated memory cache
        val = target_brightness;
        // Release locking mechanism 
        xSemaphoreGive(brightness_mutex); 
    }
    // Pass isolated snapshot upwards seamlessly
    return val;
}

/**
 * @brief FreeRTOS task that manages smooth backlight brightness transitions.
 * 
 * This task constantly runs and updates the backlight duty cycle towards the
 * target brightness to create fading effects.
 * 
 * @param pvParameters Task parameters (not used).
 */
void backlight_task(void *pvParameters) 
{
    printf("backlight_task started\n"); 
    // Declare static internal stack state variable keeping track of latest LDR math
    uint8_t new_target;
    // Commit to infinite loop explicitly necessary for standard FreeRTOS task layouts
    while (1) 
    {
        // Map inverse LDR ADC read onto valid 0-255 byte constraints
        new_target = (uint8_t)(255 - (bsp_ldr_read() * 255 / 4095)); 
        // Re-poll locked helper extracting actual current task targets
        uint8_t t = get_target_brightness(); 
        // Protect task logic from reacting to trivial ambient sensory noise (hysteresis)
        if (abs((int)new_target - (int)t) > 5) 
        {
            // If noise is significantly divergent, officially commit new ambient response
            bsp_backlight_set_target(new_target); 
            // Instantly sync localized snapshot buffer recognizing new environment goals
            t = new_target;          
        }
        // Overwrite tracked current value pushing one discrete step towards goal
        current_brightness = step_toward(current_brightness, t, RAMP_STEP);
        // Fire the newly modified step directly targeting hardware outputs
        bsp_backlight_set(current_brightness);
        // Pause this distinct task entirely checking again only after predefined ms
        vTaskDelay(pdMS_TO_TICKS(RAMP_MS));
    }                                
}

/**
 * @brief Steps a current value closer to a target value by at most a specified step size.
 * 
 * Takes care of both increasing and decreasing the value, while ensuring it never 
 * overshoots the target.
 * 
 * @param current The current value.
 * @param target The requested target value.
 * @param step The maximum change allowed in this step.
 * @return uint8_t The new value, closer to or equal to the target.
 */
static uint8_t step_toward(uint8_t current, uint8_t target, uint8_t step) 
{
    // If climbing, limit jump enforcing it never surpasses the literal ceiling
    if (current < target) return MIN(current + step, target); 
    // If descending, limit dump asserting it never cuts underneath the literal floor
    if (current > target) return MAX(current - step, target); 
    // Should values be entirely synced, return unmodified
    return current;
}                                    


/**
 * @brief Fills the entire display screen with a given color.
 * 
 * @param color 16-bit RGB565 color value.
 */
void color_screen(uint16_t color) {
    // Swap bytes for big-endian format
    uint16_t swapped = (color >> 8) | (color << 8);
    
    xSemaphoreTake(dma_done, 0);
    for (int i = 0; i < LCD_HI_RES * LCD_VE_RES; i++) {
        frame_buffer[i] = swapped;
    }
    esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, LCD_HI_RES, LCD_VE_RES, frame_buffer);
    xSemaphoreTake(dma_done, portMAX_DELAY);
}


/**
 * @brief Computes a new hue value based on a given state.
 * 
 * @param hue The current hue value.
 * @param opt The offset value to add or subtract.
 * @param state Boolean indicating whether to add (true) or subtract (false) the offset.
 * @return uint16_t The newly computed hue value.
 */
uint16_t add_state(uint16_t hue, uint16_t opt, bool state)
{
    if (state)
    {
        hue = hue + opt;
    }
    else
    {
        hue = hue - opt;
    } 
    return hue;
}

/**
 * @brief FreeRTOS task handling a display color cycling effect.
 * 
 * Cycles the display color through various sequences demonstrating DMA transfer.
 * 
 * @param pvParameters Pointer to task parameters (not used).
 */
void display_effect_task(void *pvParameters) {
    printf("display_effect_task started\n"); 
    while (1) {
        // Black to Red: 32 actual steps mapped
        for (int i = 0; i < 32; i++) {
            printf("i=%d\n", i);
            color_screen(i << 11);
            vTaskDelay(pdMS_TO_TICKS(60));
        }
        printf("BLACK TO RED\n");
        // Red to Yellow: 64 steps (Green has 6 bits in RGB565)
        for (int i = 0; i < 64; i++) {
            printf("i=%d\n", i);
            color_screen(0xF800 | (i << 5));
            vTaskDelay(pdMS_TO_TICKS(60));
        }
        printf("RED TO YELLOW\n");
        // Yellow to White: 32 steps
        for (int i = 0; i < 32; i++) {
            printf("i=%d\n", i);
            color_screen(0xFFE0 | i);
            vTaskDelay(pdMS_TO_TICKS(60));
        }
        printf("YELLOW TO WHITE\n");
        // White to Black transition
        for (int i = 0xFFFF; i >= 0; i -= 0x0841) {
            printf("i=%d\n", i);
            color_screen((uint16_t)i);
            vTaskDelay(pdMS_TO_TICKS(60));
        }
        printf("WHITE TO BLACK\n");
    }
}