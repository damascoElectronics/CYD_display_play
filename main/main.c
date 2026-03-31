#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "bsp_pinout.h"
#include "bsp.h"
#include "esp_err.h"


void app_main(void)
{
    printf("led init\n");
    init_RGB_led();

    while (1);

}
