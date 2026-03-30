#include <stdint.h>
#include <stdbool.h>
#include "bsp.h"
#include "bsp_pinout.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// funtion: GPIO initialitation 
esp_err_t init_RGB_led(void)
{
    // pin reset for RED color
    if (gpio_reset_pin(BSP_LED_R) == ESP_ERR_INVALID_ARG)
    {
        // if there is an error, return ESP_ERR_INVALID_ARG
        return ESP_ERR_INVALID_ARG;
    }
    // pin reset for GREEN color
    if (gpio_reset_pin(BSP_LED_G) == ESP_ERR_INVALID_ARG)
    {
        // if there is an error, return ESP_ERR_INVALID_ARG
        return ESP_ERR_INVALID_ARG;
    }
    // pin reset for BLUE color
    if (gpio_reset_pin(BSP_LED_B) == ESP_ERR_INVALID_ARG)
    {
        // if there is an error, return ESP_ERR_INVALID_ARG
        return ESP_ERR_INVALID_ARG;
    }


    // pin init for RED color
    if (gpio_set_direction(BSP_LED_R, GPIO_MODE_OUTPUT) == ESP_ERR_INVALID_ARG)
    {
        // if there is an error, return ESP_ERR_INVALID_ARG
        return ESP_ERR_INVALID_ARG;
    }
    // pin init for GREEN color
    if (gpio_set_direction(BSP_LED_G, GPIO_MODE_OUTPUT) == ESP_ERR_INVALID_ARG)
    {
        // if there is an error, return ESP_ERR_INVALID_ARG
        return ESP_ERR_INVALID_ARG;
    }
    // pin init for BLUE color
    if (gpio_set_direction(BSP_LED_B, GPIO_MODE_OUTPUT) == ESP_ERR_INVALID_ARG)
    {
        // if there is an error, return ESP_ERR_INVALID_ARG
        return ESP_ERR_INVALID_ARG;
    }
    // if all goes well, return ESP_OK

    ledc_timer_config_t timer = {
        .speed_mode      = LEDC_LOW_SPEED_MODE,
        .timer_num       = BSP_LED_LEDC_TIMER,
        .duty_resolution = BSP_LED_LEDC_RES,
        .freq_hz         = BSP_LED_LEDC_FREQ,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer));
    //  establized the timer
    vTaskDelay(pdMS_TO_TICKS(10));  

    gpio_num_t pins[3]         = {BSP_LED_R, BSP_LED_G, BSP_LED_B};
    ledc_channel_t channels[3] = {BSP_LED_CH_R, BSP_LED_CH_G, BSP_LED_CH_B};

    for (int i = 0; i < 3; i++) {
        ledc_channel_config_t ch = {
            .gpio_num   = pins[i],
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel    = channels[i],
            .timer_sel  = BSP_LED_LEDC_TIMER,
            .duty       = 255,
            .hpoint     = 0,
            .flags.output_invert = 0,
        };
        ESP_ERROR_CHECK(ledc_channel_config(&ch));
    }

    xTaskCreate(led_effect_task, "led_effect", 4096, NULL, 5, NULL);


    return ESP_OK;
}

esp_err_t turn_off_RGB_led(void)
{
    // off RED color
    if (gpio_set_level(BSP_LED_R, 1) == ESP_ERR_INVALID_ARG)
    {
        // if there is an error, return ESP_ERR_INVALID_ARG
        return ESP_ERR_INVALID_ARG;
    }
    // off GREEN color
    if (gpio_set_level(BSP_LED_G, 1) == ESP_ERR_INVALID_ARG)
    {
        // if there is an error, return ESP_ERR_INVALID_ARG
        return ESP_ERR_INVALID_ARG;
    }
    // off BLUE color
    if (gpio_set_level(BSP_LED_B, 1) == ESP_ERR_INVALID_ARG)
    {
        // if there is an error, return ESP_ERR_INVALID_ARG
        return ESP_ERR_INVALID_ARG;
    }
    // if all goes well, return ESP_OK
    return ESP_OK;   
}

esp_err_t turn_on_RGB_led(void)
{
    // off RED color
    if (gpio_set_level(BSP_LED_R, 0) == ESP_ERR_INVALID_ARG)
    {
        // if there is an error, return ESP_ERR_INVALID_ARG
        return ESP_ERR_INVALID_ARG;
    }
    // off GREEN color
    if (gpio_set_level(BSP_LED_G, 0) == ESP_ERR_INVALID_ARG)
    {
        // if there is an error, return ESP_ERR_INVALID_ARG
        return ESP_ERR_INVALID_ARG;
    }
    // off BLUE color
    if (gpio_set_level(BSP_LED_B, 0) == ESP_ERR_INVALID_ARG)
    {
        // if there is an error, return ESP_ERR_INVALID_ARG
        return ESP_ERR_INVALID_ARG;
    }
    // if all goes well, return ESP_OK
    return ESP_OK;   
}


esp_err_t control_single_color_RGB_led(gpio_num_t gpio_num, bool level)
{
    return gpio_set_level(gpio_num, !level);  
}


void bsp_led_set(uint8_t r, uint8_t g, uint8_t b) {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, BSP_LED_CH_R, 255 - r);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, BSP_LED_CH_G, 255 - g);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, BSP_LED_CH_B, 255 - b);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, BSP_LED_CH_R);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, BSP_LED_CH_G);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, BSP_LED_CH_B);
}

void led_effect_task(void *pvParameters) {
    uint8_t hue = 0;
    bool dir = true;
    while (1) {
        bsp_led_set(hue, 255 - hue, 128);
        
        switch (hue)
        {
            case 0:
                dir = true;
                break;
            case 255:
                dir = false;
                break;
            default:
                break;
        }

        if (dir)
        {
            hue++;
        }
        else{
            hue--;
            
        }
        
        
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}