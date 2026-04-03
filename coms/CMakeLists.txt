idf_component_register(
    SRCS "src/i2c_bus.c" "src/spi_bus.c" "src/uart_bus.c"
    INCLUDE_DIRS "inc"
    REQUIRES esp_lcd driver espressif__esp_lcd_ili9341
)
