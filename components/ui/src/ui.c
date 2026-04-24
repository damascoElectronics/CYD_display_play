/**
 * @file ui.c
 * @brief UI layer initialization and public API implementation.
 */
#include "ui.h"
#include "screen_main.h"
#include "esp_lvgl_port.h"

esp_err_t ui_init(void)
{
    lvgl_port_lock(0);
    screen_main_create();
    lvgl_port_unlock();
    return ESP_OK;
}

void ui_set_frequency(float freq)      { screen_main_set_frequency(freq); }
void ui_set_station_name(const char *n){ screen_main_set_station_name(n); }
void ui_set_signal_strength(uint8_t l) { screen_main_set_signal(l); }
void ui_set_time(uint8_t h, uint8_t m, uint8_t s) { screen_main_set_time(h, m, s); }
void ui_set_country(const char *c)     { screen_main_set_country(c); }