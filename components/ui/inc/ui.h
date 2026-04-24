/**
 * @file ui.h
 * @brief Public API for the CYD Radio UI layer.
 */
#ifndef UI_H
#define UI_H

#include "esp_err.h"
#include <stdint.h>

/**
 * @brief Initializes the UI and displays the main screen.
 *
 * @return esp_err_t ESP_OK on success.
 */
esp_err_t ui_init(void);

/**
 * @brief Updates the frequency display on the main screen.
 *
 * @param freq Frequency in MHz (e.g. 98.7).
 */
void ui_set_frequency(float freq);

/**
 * @brief Updates the station name label on the main screen.
 *
 * @param name Station name string (RDS or manual).
 */
void ui_set_station_name(const char *name);

/**
 * @brief Updates the signal strength bar on the main screen.
 *
 * @param level Signal level 0-100.
 */
void ui_set_signal_strength(uint8_t level);

/**
 * @brief Updates the clock display on the top bar.
 *
 * @param h Hours (0-23).
 * @param m Minutes (0-59).
 * @param s Seconds (0-59).
 */
void ui_set_time(uint8_t h, uint8_t m, uint8_t s);

/**
 * @brief Updates the country label on the top bar.
 *
 * @param country Country code string (e.g. "LUX").
 */
void ui_set_country(const char *country);

#endif