idf_component_register(
    SRCS "src/i2c_bus.c" "src/spi_bus.c" "src/uart_bus.c"
    INCLUDE_DIRS "inc"
    REQUIRES esp_lcd driver esp_driver_gpio  esp_driver_spi esp_driver_i2c esp_driver_uart
)
