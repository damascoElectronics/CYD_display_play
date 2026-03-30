#ifndef BSP_H
#define BSP_H

#include "esp_err.h"
#include "driver/ledc.h"

// timer 

// RGB
#define BSP_LED_LEDC_TIMER    LEDC_TIMER_1        // timer 0 ya lo usa el backlight
#define BSP_LED_LEDC_RES      LEDC_TIMER_8_BIT    // 0-255
#define BSP_LED_LEDC_FREQ     5000

#define BSP_LED_CH_R  LEDC_CHANNEL_1
#define BSP_LED_CH_G  LEDC_CHANNEL_2
#define BSP_LED_CH_B  LEDC_CHANNEL_3

#define BSP_TFT_BL          21
#define BSP_BL_LEDC_TIMER   LEDC_TIMER_0
#define BSP_BL_LEDC_CHANNEL LEDC_CHANNEL_0
#define BSP_BL_LEDC_FREQ    5000    // Hz
#define BSP_BL_LEDC_RES     LEDC_TIMER_8_BIT  // 0-255

esp_err_t init_RGB_led(void);
esp_err_t turn_off_RGB_led(void);
esp_err_t turn_on_RGB_led(void);
void bsp_led_set(uint8_t r, uint8_t g, uint8_t b);
void led_effect_task(void *pvParameters);

#endif