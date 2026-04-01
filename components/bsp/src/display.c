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


/** @brief Mutex to protect multithreaded access to brightness variables */
static SemaphoreHandle_t brightness_mutex = NULL;
/** @brief Current brightness level of the backlight (0-255) */
static uint8_t current_brightness = 0;
/** @brief Target brightness level of the backlight to transition to (0-255) */
static uint8_t target_brightness  = 255;
/** @brief Amount to change the brightness per step */
#define RAMP_STEP  2      // how much it changes per tick
/** @brief Interval between brightness changes in milliseconds */
#define RAMP_MS    20     // how often do the changes in ms


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
    // pin reset for backlight  
    if (gpio_reset_pin(BSP_TFT_BL) == ESP_ERR_INVALID_ARG)
    {
        // if there is an error, return ESP_ERR_INVALID_ARG
        return ESP_ERR_INVALID_ARG;
    }
    
    // pin init for backlight  
    if (gpio_set_direction(BSP_TFT_BL, GPIO_MODE_OUTPUT) == ESP_ERR_INVALID_ARG)
    {
        // if there is an error, return ESP_ERR_INVALID_ARG
        return ESP_ERR_INVALID_ARG;
    }
    
    ledc_timer_config_t timer = {
        .speed_mode      = LEDC_LOW_SPEED_MODE,
        .timer_num       = BSP_BL_LEDC_TIMER,
        .duty_resolution = BSP_BL_LEDC_RES,
        .freq_hz         = BSP_BL_LEDC_FREQ,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer));
    //  establized the timer
    vTaskDelay(pdMS_TO_TICKS(10));  

    ledc_channel_config_t ch = {
        .gpio_num   = BSP_TFT_BL,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = BSP_BL_LEDC_CHANNEL,
        .timer_sel  = BSP_BL_LEDC_TIMER,
        .duty       = 255,
        .hpoint     = 0,
        .flags.output_invert = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ch));

    brightness_mutex = xSemaphoreCreateMutex();
    // creating and starting task
    xTaskCreate(backlight_task, "backlight", 2048, NULL, 0, NULL);
    bsp_backlight_set_target(120);
    // if all goes well, return ESP_OK
    return ESP_OK;
}


/**
 * @brief Sets the duty cycle for the backlight screen via LEDC.
 * 
 * @param backlight backlight duty cycle value (0-255).
 */
void bsp_backlight_set(uint8_t backlight) {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, BSP_BL_LEDC_CHANNEL, backlight);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, BSP_BL_LEDC_CHANNEL);
}


/**
 * @brief Sets the target brightness for the backlight.
 * 
 * The backlight_task will smoothly transition to this target brightness.
 * 
 * @param brightness Target brightness level (0-255).
 */
void bsp_backlight_set_target(uint8_t brightness) {
    if (xSemaphoreTake(brightness_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        target_brightness = brightness;
        xSemaphoreGive(brightness_mutex);
    }
}

/**
 * @brief Thread-safe getter for the target brightness.
 * 
 * @return uint8_t Current target brightness level.
 */
static uint8_t get_target_brightness(void) {
    uint8_t val = 255;
    if (xSemaphoreTake(brightness_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        val = target_brightness;
        xSemaphoreGive(brightness_mutex);
    }
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
void backlight_task(void *pvParameters) {

    uint8_t new_target;

    while (1) {
        new_target = (uint8_t)(255 - (bsp_ldr_read() * 255 / 4095));
        if (abs((int)new_target - (int)get_target_brightness()) > 5) 
        {
            bsp_backlight_set_target(new_target);
        }
        printf("new_target: %d\n",new_target);

        if (current_brightness < target_brightness) {
            current_brightness += RAMP_STEP;
            if (current_brightness > target_brightness)
                current_brightness = target_brightness;
        } else if (current_brightness > target_brightness) {
            current_brightness -= RAMP_STEP;
            if (current_brightness < target_brightness)
                current_brightness = target_brightness;
        }

        bsp_backlight_set(current_brightness);
        vTaskDelay(pdMS_TO_TICKS(RAMP_MS));
    }
}