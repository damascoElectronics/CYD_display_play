/**
 * @file main.c
 * @brief Main application entry point for the CYD display project.
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "bsp_pinout.h"
#include "bsp.h"
#include "esp_err.h"

/**
 * @brief Main application entry point.
 * 
 * Initializes the RGB LED and the display.
 */
void app_main(void)
{
    printf("init program\n");
    init_RGB_led();
    init_display();
}
