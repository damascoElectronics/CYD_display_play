#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

void app_main(void)
{
    printf("led \n");
    // Backlight ON
    gpio_reset_pin(BSP_LED_R);
    gpio_reset_pin(BSP_LED_G);
    gpio_reset_pin(BSP_LED_B);

    gpio_set_direction(BSP_LED_R, GPIO_MODE_OUTPUT);
    gpio_set_direction(BSP_LED_G, GPIO_MODE_OUTPUT);
    gpio_set_direction(BSP_LED_B, GPIO_MODE_OUTPUT);

    gpio_set_level(BSP_LED_R, 1);
    gpio_set_level(BSP_LED_G, 1);
    gpio_set_level(BSP_LED_B, 1);


    printf("Backlight ON\n");

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        gpio_set_level(BSP_LED_B, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));
        gpio_set_level(BSP_LED_B, 1);
        

        

    }

}
