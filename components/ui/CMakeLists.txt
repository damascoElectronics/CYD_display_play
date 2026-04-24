idf_component_register(
    SRCS "src/ui.c" "src/screens/screen_main.c"
    INCLUDE_DIRS "inc"
    REQUIRES bsp lvgl__lvgl espressif__esp_lvgl_port
)