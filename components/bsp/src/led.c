/**
 * @file led.c
 * @brief RGB LED control and effect implementations.
 */
#include <stdint.h>
#include <stdbool.h>
#include "bsp.h"
#include "bsp_pinout.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/**
 * @brief Heartbeat curve representing PWM values (0-255).
 * 
 * Contains a sequence to simulate two quick peaks followed by a pause.
 */
static const uint8_t heartbeat_curve[] = {
    0,  10,  30,  60, 100, 150, 200, 240, 255, 220,  // peak rise 1
  170, 110,  60,  80, 120, 170, 210, 240, 220, 180,  // descent + ascent peak 2
  130,  80,  40,  15,   5,   0,   0,   0,   0,   0,  // down + pause
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // long pause
};

/** @brief Total number of steps in the heartbeat curve array */
#define HEARTBEAT_STEPS  (sizeof(heartbeat_curve) / sizeof(heartbeat_curve[0]))

/**
 * @brief Initializes the RGB LED GPIO and LEDC timer/channel configurations.
 * 
 * Configures the GPIO direction and sets up the LEDC timer and channels 
 * for PWM control of the RGB LED. Also spawns the led_effect_task.
 *
 * @return esp_err_t ESP_OK on success, or ESP_ERR_INVALID_ARG on failure.
 */
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
    // turn off before start to use.
    turn_off_RGB_led();
    // if all goes well, return ESP_OK

    return ESP_OK;
}

/**
 * @brief Turns off the RGB LED by setting the respective GPIO levels to High (off state).
 * 
 * @return esp_err_t ESP_OK on success, or ESP_ERR_INVALID_ARG upon failure.
 */
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

/**
 * @brief Turns on the RGB LED by setting the respective GPIO levels to Low (on state).
 * 
 * @return esp_err_t ESP_OK on success, or ESP_ERR_INVALID_ARG upon failure.
 */
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


/**
 * @brief Controls a single color channel of the RGB LED.
 * 
 * @param gpio_num The GPIO number of the color channel.
 * @param level The desired logical level (true for ON, false for OFF).
 * @return esp_err_t ESP_OK on success, or ESP_ERR_INVALID_ARG upon failure.
 */
esp_err_t control_single_color_RGB_led(gpio_num_t gpio_num, bool level)
{
    return gpio_set_level(gpio_num, !level);  
}


/**
 * @brief Sets the duty cycle for the RGB LED via LEDC.
 * 
 * @param r Red duty cycle value (0-255).
 * @param g Green duty cycle value (0-255).
 * @param b Blue duty cycle value (0-255).
 */
void bsp_led_set(uint8_t r, uint8_t g, uint8_t b) {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, BSP_LED_CH_R, 255 - r);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, BSP_LED_CH_G, 255 - g);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, BSP_LED_CH_B, 255 - b);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, BSP_LED_CH_R);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, BSP_LED_CH_G);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, BSP_LED_CH_B);
}

/**
 * @brief Call this function periodically (e.g., every 20ms) to 
 *        obtain the next PWM value of the heartbeat effect.
 *
 * @return uint8_t  PWM value [0-255]
 */
uint8_t heartbeat_tick(void)
{
    static uint16_t step = 0;

    uint8_t pwm_val = heartbeat_curve[step];

    step++;
    if (step >= HEARTBEAT_STEPS)
        step = 0;

    return pwm_val;
}


/**
 * @brief FreeRTOS task handling a heartbeat effect for the RGB LED.
 * 
 * Cycles the heartbeat pattern through the Red, Green, and Blue channels.
 * 
 * @param pvParameters Pointer to task parameters (not used).
 */
void led_effect_task(void *pvParameters) {
    uint8_t hue = 0;
    uint8_t count = 0;
    uint8_t dir = 1;

    while (1) {
        hue = heartbeat_tick();
        if (count >= HEARTBEAT_STEPS)
        {
            count = 0;
            dir = dir << 1;
        }

        switch (dir)
        {
        case 1:
            bsp_led_set(hue, 0, 0);
            break;
        case 2:
            bsp_led_set(0, hue, 0);
            break;
        case 4:
            bsp_led_set(0, 0, hue);
            break;
        case 8:
            dir = 1;
            break;
        default:
            break;
        }
        count ++;
        vTaskDelay(pdMS_TO_TICKS(30));
    }
}   