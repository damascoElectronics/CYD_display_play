idf_component_register(
    SRCS "src/led.c" "src/display.c" "src/sensors.c"
    INCLUDE_DIRS "inc"
    REQUIRES esp_driver_gpio  esp_driver_ledc esp_adc esp_lcd
)