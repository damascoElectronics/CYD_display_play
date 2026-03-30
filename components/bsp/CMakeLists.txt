idf_component_register(
    SRCS "src/led.c"
    INCLUDE_DIRS "inc"
    REQUIRES esp_driver_gpio  esp_driver_ledc 
)