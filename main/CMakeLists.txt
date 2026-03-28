idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "." "inc"
                    REQUIRES esp_lcd driver espressif__esp_lcd_ili9341)