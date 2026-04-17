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

esp_lcd_panel_handle_t panel_handle = NULL;
esp_lcd_panel_io_handle_t io_handle = NULL;

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
 * @brief Initializes the display and configure it for its use.
 * 
 * Configures the screen to it use.
 * 
 *
 * @return esp_err_t ESP_OK on success, or ESP_ERR_INVALID_ARG on failure.
 */
esp_err_t init_display(void)
{
    // Call ESP hardware layer to reset config on backlight pin
    if (gpio_reset_pin(BSP_TFT_BL) == ESP_ERR_INVALID_ARG) 
    {
        // If reset fails due to invalid parameters: return ESP_ERR_INVALID_ARG
        return ESP_ERR_INVALID_ARG;
    }
    
    // Request ESP config layer direct backlight pin out
    if (gpio_set_direction(BSP_TFT_BL, GPIO_MODE_OUTPUT) == ESP_ERR_INVALID_ARG)
    {
        // If configuration rejects invalid parameters: return ESP_ERR_INVALID_ARG
        return ESP_ERR_INVALID_ARG;
    }
    // Instantiate and fill an LED Controller timer config packet
    ledc_timer_config_t timer = {    
        .speed_mode      = LEDC_LOW_SPEED_MODE, // Assure timer runs in simple low speed mode
        .timer_num       = BSP_BL_LEDC_TIMER,   // Link timer to the enumerated BSP timer slot
        .duty_resolution = BSP_BL_LEDC_RES,     // Assign the max resolution to defined bounds
        .freq_hz         = BSP_BL_LEDC_FREQ,    // Assign PWM frequency equal to standard defaults
        .clk_cfg         = LEDC_AUTO_CLK,       // Let subsystem automatically assign the best stable system clock
    };
    // Apply constructed timer profile checking hardware bounds automatically
    ESP_ERROR_CHECK(ledc_timer_config(&timer));
    // Give internal hardware states 10 milliseconds to physically latch logic levels
    vTaskDelay(pdMS_TO_TICKS(10));
    // Instantiate and fill an LED Controller channel config packet connecting specific GPIO
    ledc_channel_config_t ch = {
        .gpio_num   = BSP_TFT_BL,           // Select physical pin bound mapped for TFT backlight
        .speed_mode = LEDC_LOW_SPEED_MODE,  // Synchronize channel output speed mapping
        .channel    = BSP_BL_LEDC_CHANNEL,  // Hook config logic to targeted channel hardware generator
        .timer_sel  = BSP_BL_LEDC_TIMER,    // Hook channel profile explicitly to customized running timer
        .duty       = 255,                  // Immediately force maximum duty cycle logic onto pin buffer (full power startup)
        .hpoint     = 0,                    // Start logical pulse completely at the beginning of the clock step
        .flags.output_invert = 0,           // Ensure logical low is physical 0v, logical high is physical 3.3v
    };
    // Force iteration explicitly checking against underlying system boundaries assuring total safety
    ESP_ERROR_CHECK(ledc_channel_config(&ch)); 
    // Instantiate FreeRTOS mutex semaphore to safely shield future concurrent IO attempts
    brightness_mutex = xSemaphoreCreateMutex(); 
    // Launch parallel FreeRTOS agent running dynamic fading
    xTaskCreate(backlight_task, "backlight", 2048, NULL, 0, NULL); 
    // Enqueue system fading goal explicitly targeting 120 (mid-scale glow)
    bsp_backlight_set_target(120);
   
   // LCD IO
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = BSP_TFT_DC,
        .cs_gpio_num = BSP_TFT_CS,
        .pclk_hz = 40 * 1000 * 1000,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    esp_lcd_new_panel_io_spi(SPI2_HOST, &io_config, &io_handle);
   
    // Panel ILI9341
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = BSP_TFT_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
    };
    esp_lcd_new_panel_ili9341(io_handle, &panel_config, &panel_handle);

    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);
    esp_lcd_panel_swap_xy(panel_handle, true);
    esp_lcd_panel_mirror(panel_handle, true, false);
    esp_lcd_panel_disp_on_off(panel_handle, true);

    xTaskCreate(display_effect_task, "display_effect", 2048, NULL, 0, NULL);

   
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

void color_screen(uint16_t color)
{
    uint16_t *color_buf = heap_caps_malloc(LCD_HI_RES * sizeof(uint16_t), MALLOC_CAP_DMA);
    for (int i = 0; i < LCD_HI_RES; i++)  // rojo en RGB565
    {
        color_buf[i] = color;
    }
    for (int y = 0; y < LCD_VE_RES; y++) 
    {
        esp_lcd_panel_draw_bitmap(panel_handle, 0, y, LCD_HI_RES, y + 1, color_buf);
    }
    printf("Pantalla roja!\n");
    free(color_buf);
}


void display_effect_task(void *pvParameters) {
    uint16_t hue = 0;
    
    while (1) 
    {
        color_screen(hue);
        hue++;
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}