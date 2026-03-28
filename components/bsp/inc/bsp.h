#ifndef BPS_H
#define BPS_H

// display resolution
#define LCD_H_RES 320
#define LCD_V_RES 240

#define BSP_TFT_BL          21
#define BSP_BL_LEDC_TIMER   LEDC_TIMER_0
#define BSP_BL_LEDC_CHANNEL LEDC_CHANNEL_0
#define BSP_BL_LEDC_FREQ    5000    // Hz
#define BSP_BL_LEDC_RES     LEDC_TIMER_8_BIT  // 0-255

uint8_t bsp_backlight_set(uint8_t brightness);

#endif



