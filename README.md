# CYD Radio — ESP32 Cheap Yellow Display (ESP32-2432S028R)

Full FM/AM radio with touchscreen UI based on the CYD board and the TEF6686 tuner.

## Hardware

| Component | Details |
|-----------|---------|
| Board | ESP32-2432S028R (Cheap Yellow Display) |
| Tuner | TEF6686 (FM/AM/SW/LW, UART) — pending |
| Audio | SC8002B onboard amplifier — pending |
| Speaker | P4 onboard connector — pending |

## Stack

| Layer | Technology |
|-------|------------|
| Framework | ESP-IDF v6.0 |
| IDE | VSCode + ESP-IDF Extension |
| RTOS | FreeRTOS |
| Display driver | `esp_lcd` + `espressif/esp_lcd_ili9341` v2.0.2 |
| UI | LVGL v9 via `espressif/esp_lvgl_port` v2.3.x |
| Color format | RGB565 with `swap_bytes = true` |

## Dependencies

Go to `./main/idf_component.yml`

> **Note:** `lv_conf.h` must be manually copied to `managed_components/lvgl__lvgl/lv_conf.h` after `idf.py reconfigure`. 

## Important Notes

- Backlight (GPIO 21) controlled by LEDC Timer 0 with smooth ramp via FreeRTOS task
- RGB LED uses LEDC Timer 1, channels 1-3, common anode (inverted logic)
- LDR on GPIO 34 — ADC1 channel 6. Hardware possibly defective on some units, pending diagnosis with multimeter
- SPI clock reduced to 20MHz to lower power consumption (target: battery operation)
- TEF6686 will connect via UART2 on GPIO 22/27 to avoid interference with UART0 (debug)
- SD card shares VSPI bus with touch (GPIO 32/39/25), CS on GPIO 5

## UI — Main Screen

![alt text](https://github.com/damascoElectronics/CYD_display_play/blob/main/documentation/images/Screenshot%202026-04-24%20102428.png)
![alt text](https://github.com/damascoElectronics/CYD_display_play/blob/main/documentation/images/Screenshot%202026-04-24%20102510.png)


- Central display toggles between frequency (7-seg style) and spectrum analyzer on tap
- 3x4 numpad: manual frequency entry
- Presets 1-5: saved stations stored in NVS
- SCAN: automatic station search and save

## Progress

- [x] ESP-IDF v6.0 toolchain configured
- [x] Backlight PWM with smooth ramp (LEDC + FreeRTOS)
- [x] Automatic brightness adjustment via LDR (EMA + averaging) — pending hardware diagnosis
- [x] ILI9341 display initialized via `esp_lcd`
- [x] Color fill and screen effects working
- [x] RGB LED with heartbeat effect (LEDC + FreeRTOS)
- [x] LVGL v9 initialized via `esp_lvgl_port`
- [x] LVGL text rendering clear and well-defined
- [ ] XPT2046 touch
- [ ] Main screen UI (LVGL)
- [ ] Presets stored in NVS
- [ ] TEF6686 driver (UART2)
- [ ] SC8002B audio + speaker
- [ ] SD card — RGB565 image loading
- [ ] DCF77 / clock via TEF6686 LW
- [ ] Low power mode (light sleep between interactions)
- [ ] Settings screen
