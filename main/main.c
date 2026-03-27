#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define TFT_BL 21

void app_main(void)
{
    printf("CYD iniciando...\n");

    // Backlight ON
    gpio_reset_pin(TFT_BL);
    gpio_set_direction(TFT_BL, GPIO_MODE_OUTPUT);

    while (1) {
        gpio_set_level(TFT_BL, 1);
        printf("Backlight ON\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
        printf("Backlight OFF\n");
        gpio_set_level(TFT_BL, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));

        
    }
}