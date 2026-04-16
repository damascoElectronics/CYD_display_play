#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "spi_bus.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"


/**
 * @brief Initializes the SPI bus and configure it for its use.
 * 
 * @return esp_err_t ESP_OK on success, or ESP_ERR_INVALID_ARG on failure.
 */
esp_err_t init_spi(void)
{
    spi_bus_config_t buscfg = {
        .miso_io_num = TFT_MISO,
        .mosi_io_num = TFT_MOSI,
        .sclk_io_num = TFT_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_H_RES * LCD_V_RES * 2,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    vTaskDelay(pdMS_TO_TICKS(10));
    return ESP_OK;                   

}