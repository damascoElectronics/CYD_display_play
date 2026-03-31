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




/**
 * @brief Initializes the display and configure it for its use.
 * 
 * Configures the screen to it use.
 * 
 *
 * @return esp_err_t ESP_OK on success, or ESP_ERR_INVALID_ARG on failure.
 */
esp_err_t init_display_led(void)
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

    bsp_backlight_set(0);

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
