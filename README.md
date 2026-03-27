# CYD - ESP32 Cheap Yellow Display (ESP32-2432S028R)

Development project for the devkit ESP32-2432S028R using only ESP-IDF and VScode.

## Hardware

- **Board:** ESP32-2432S028R (Cheap Yellow Display)
- **Display:** 2.8" TFT ILI9341 — 240x320px resistive touchscreen
- **MCU:** ESP32-WROOM-32 (dual core, 240MHz, 4MB Flash)

## Pinout

| Funtion       | GPIO |
|---------------|------|
| TFT MISO      | 12   |
| TFT MOSI      | 13   |
| TFT SCLK      | 14   |
| TFT CS        | 15   |
| TFT DC        | 2    |
| TFT RST       | -1   |
| TFT Backlight | 21   |
| Touch CS      | 33   |
| Touch IRQ     | 36   |
| Touch MOSI    | 32   |
| Touch MISO    | 39   |
| Touch CLK     | 25   |
| RGB LED R     | 4    |
| RGB LED G     | 16   |
| RGB LED B     | 17   |

## Stack

- **Framework:** ESP-IDF v6.0
- **IDE:** VSCode + ESP-IDF Extension
- **Display driver:** `espressif/esp_lcd_ili9341` v2.0.2
- **Color order:** `LCD_RGB_ELEMENT_ORDER_RGB`

## Notes

- Backlight (GPIO 21) has to be turn on manualy.
- Touch use SPI bus split with non_estandar pins  (VSPI).

## Progreso

- [x] Toolchain ESP-IDF v6.0 configurated
- [x] Backlight working
- [x] Display ILI9341 inicialitated via `esp_lcd`
- [x] Color fill working (red, gree, blue)
- [ ] Touch XPT2046
- [ ] drawing basic colors