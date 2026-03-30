/**
 * @file bsp.h
 * @brief Board Support Package API declarations.
 */
#ifndef BSP_H
#define BSP_H

#include "esp_err.h"
#include "driver/ledc.h"

// timer 

// RGB
/** @brief Timer used for RGB LED (Timer 0 is used by backlight) */
#define BSP_LED_LEDC_TIMER    LEDC_TIMER_1        // timer 0 ya lo usa el backlight
/** @brief Resolution for RGB LED timer (0-255) */
#define BSP_LED_LEDC_RES      LEDC_TIMER_8_BIT    // 0-255
/** @brief Frequency for RGB LED timer */
#define BSP_LED_LEDC_FREQ     5000

/** @brief LEDC channel for Red LED */
#define BSP_LED_CH_R  LEDC_CHANNEL_1
/** @brief LEDC channel for Green LED */
#define BSP_LED_CH_G  LEDC_CHANNEL_2
/** @brief LEDC channel for Blue LED */
#define BSP_LED_CH_B  LEDC_CHANNEL_3

/** @brief TFT Backlight GPIO pin */
#define BSP_TFT_BL          21
/** @brief Timer used for TFT Backlight */
#define BSP_BL_LEDC_TIMER   LEDC_TIMER_0
/** @brief LEDC channel for TFT Backlight */
#define BSP_BL_LEDC_CHANNEL LEDC_CHANNEL_0
/** @brief Frequency for TFT Backlight timer (Hz) */
#define BSP_BL_LEDC_FREQ    5000    // Hz
/** @brief Resolution for TFT Backlight timer (0-255) */
#define BSP_BL_LEDC_RES     LEDC_TIMER_8_BIT  // 0-255

/**
 * @brief Initializes the RGB LED setup.
 * 
 * @return esp_err_t ESP_OK on success.
 */
esp_err_t init_RGB_led(void);

/**
 * @brief Turns off the RGB LED.
 * 
 * @return esp_err_t ESP_OK on success.
 */
esp_err_t turn_off_RGB_led(void);

/**
 * @brief Turns on the RGB LED.
 * 
 * @return esp_err_t ESP_OK on success.
 */
esp_err_t turn_on_RGB_led(void);

/**
 * @brief Sets the duty cycle for the RGB LED channels.
 * 
 * @param r Red value (0-255).
 * @param g Green value (0-255).
 * @param b Blue value (0-255).
 */
void bsp_led_set(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief FreeRTOS task that runs a lighting effect on the RGB LED.
 * 
 * @param pvParameters Task parameters (not used).
 */
void led_effect_task(void *pvParameters);

#endif