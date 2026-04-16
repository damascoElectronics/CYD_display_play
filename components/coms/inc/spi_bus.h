/**
 * @file spi_bus.h
 * @brief .
 */
#ifndef SPI_BUS_H
#define SPI_BUS_H

#define TFT_BL   21
#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST  -1


#define LCD_H_RES 320
#define LCD_V_RES 240

/**
 * @brief Initializes the SPI bus and configure it for its use.
 * 
 * @return esp_err_t ESP_OK on success, or ESP_ERR_INVALID_ARG on failure.
 */
esp_err_t init_spi(void);


#endif