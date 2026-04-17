/**
 * @file bsp.h
 * @brief Board Support Package API declarations.
 */
#ifndef BSP_H
#define BSP_H

#include "driver/ledc.h"
#include "esp_err.h"


// timer

// RGB
/** @brief Timer used for RGB LED (Timer 0 is used by backlight) */
#define BSP_LED_LEDC_TIMER LEDC_TIMER_1 // timer 0 is already used by backlight
/** @brief Resolution for RGB LED timer (0-255) */
#define BSP_LED_LEDC_RES LEDC_TIMER_8_BIT // 0-255
/** @brief Frequency for RGB LED timer */
#define BSP_LED_LEDC_FREQ 5000

/** @brief LEDC channel for Red LED */
#define BSP_LED_CH_R LEDC_CHANNEL_1
/** @brief LEDC channel for Green LED */
#define BSP_LED_CH_G LEDC_CHANNEL_2
/** @brief LEDC channel for Blue LED */
#define BSP_LED_CH_B LEDC_CHANNEL_3

/** @brief TFT Backlight GPIO pin */
#define BSP_TFT_BL 21
/** @brief TFT  GPIO Chip Selectrt pin */
#define BSP_TFT_CS   15
/** @brief TFT GPIO Data/Command */
#define BSP_TFT_DC    2
/** @brief TFT LCD rest command */
#define BSP_TFT_RST  -1

/** @brief Timer used for TFT Backlight */
#define BSP_BL_LEDC_TIMER LEDC_TIMER_0
/** @brief LEDC channel for TFT Backlight */
#define BSP_BL_LEDC_CHANNEL LEDC_CHANNEL_0
/** @brief Frequency for TFT Backlight timer (Hz) */
#define BSP_BL_LEDC_FREQ 5000 // Hz
/** @brief Resolution for TFT Backlight timer (0-255) */
#define BSP_BL_LEDC_RES LEDC_TIMER_8_BIT // 0-255
/** @brief ADC channel for LDR sensor */
#define BSP_LDR_CHANNEL ADC_CHANNEL_6
/** @brief ADC uint for LDR sensort */
#define BSP_LDR_UNIT ADC_UNIT_1

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
 *
 * @return esp_err_t ESP_OK on success.
 */
esp_err_t bsp_led_set(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief FreeRTOS task that runs a lighting effect on the RGB LED.
 *
 * @param pvParameters Task parameters (not used).
 */
void led_effect_task(void *pvParameters);

/**
 * @brief Initializes the display and configure it for its use.
 *
 * Configures the screen to it use.
 *
 *
 * @return esp_err_t ESP_OK on success, or ESP_ERR_INVALID_ARG on failure.
 */
esp_err_t init_display(void);

/**
 * @brief Sets the duty cycle for the backlight screen via LEDC.
 *
 * @param backlight backlight duty cycle value (0-255).
 */
void bsp_backlight_set(uint8_t backlight);

/**
 * @brief FreeRTOS task that manages smooth backlight brightness transitions.
 *
 * @param pvParameters Task parameters (not used).
 */
void backlight_task(void *pvParameters);

/**
 * @brief Sets the target brightness for the backlight.
 *
 * The backlight task will smoothly transition to this target brightness.
 *
 * @param brightness Target brightness level (0-255).
 */
void bsp_backlight_set_target(uint8_t brightness);

/**
 * @brief Initializes the LDR (Light Dependent Resistor) sensor ADC.
 * 
 * @return esp_err_t ESP_OK on success.
 */
esp_err_t bsp_ldr_init(void);

/**
 * @brief Reads the exponentially smoothed value of the LDR sensor.
 * 
 * @return int The smoothed LDR raw ADC value (0-4095).
 */
int bsp_ldr_read(void);

/**
 * @brief Reads the LDR sensor voltage utilizing calibration.
 * 
 * @return int Voltage in millivolts (mV).
 */
int bsp_ldr_read_mv(void);

#endif