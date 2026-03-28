#include "driver/spi_master.h"

#include "spi.h"

int8_t spi_bus_init(void) 
{
    // SPI bus
    spi_bus_config_t buscfg = {
        .miso_io_num = BSP_TFT_MISO,
        .mosi_io_num = BSP_TFT_MOSI,
        .sclk_io_num = BSP_TFT_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_H_RES * LCD_V_RES * 2,
    };
    spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    return 0;
}