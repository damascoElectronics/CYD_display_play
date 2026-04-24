/**
 * @file screen_main.h
 * @brief Main radio screen — layout and widget update API.
 */
#ifndef SCREEN_MAIN_H
#define SCREEN_MAIN_H

#include "lvgl.h"
#include <stdint.h>

/**
 * @brief Creates and displays the main radio screen.
 *
 * Builds the full layout: top bar, display area, numpad, presets, footer.
 */
void screen_main_create(void);

/**
 * @brief Updates the frequency label.
 * @param freq Frequency in MHz.
 */
void screen_main_set_frequency(float freq);

/**
 * @brief Updates the station name label.
 * @param name Station name string.
 */
void screen_main_set_station_name(const char *name);

/**
 * @brief Updates the signal strength bar.
 * @param level 0-100.
 */
void screen_main_set_signal(uint8_t level);

/**
 * @brief Updates the clock label on the top bar.
 * @param h Hours. @param m Minutes. @param s Seconds.
 */
void screen_main_set_time(uint8_t h, uint8_t m, uint8_t s);

/**
 * @brief Updates the country label on the top bar.
 * @param country Country code string.
 */
void screen_main_set_country(const char *country);

#endif