idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES bsp coms esp_driver_gpio
                    )