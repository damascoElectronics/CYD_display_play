/**
 * @file screen_main.c
 * @brief Main radio screen layout and widget implementations.
 */
#include "screen_main.h"
#include "esp_lvgl_port.h"
#include <stdio.h>

// ─── Widget handles ───────────────────────────────────────────────────────────

static lv_obj_t *lbl_clock      = NULL;
static lv_obj_t *lbl_station    = NULL;
static lv_obj_t *lbl_country    = NULL;
static lv_obj_t *lbl_frequency  = NULL;
static lv_obj_t *lbl_band       = NULL;
static lv_obj_t *bar_signal     = NULL;
static lv_obj_t *cont_display   = NULL;  // toggleable display area

// ─── Layout constants ─────────────────────────────────────────────────────────

#define SCREEN_W        320
#define SCREEN_H        240
#define TOPBAR_H        26
#define FOOTER_H        16
#define PRESET_H        20
#define DISPLAY_W       188
#define PAD_W           (SCREEN_W - DISPLAY_W - 8)  // right pad width
#define MAIN_H          (SCREEN_H - TOPBAR_H - FOOTER_H - PRESET_H - 6)

// ─── Internal helpers ─────────────────────────────────────────────────────────

static void build_top_bar(lv_obj_t *parent);
static void build_display_area(lv_obj_t *parent);
static void build_numpad(lv_obj_t *parent);
static void build_preset_bar(lv_obj_t *parent);
static void build_footer(lv_obj_t *parent);

// ─── Public ───────────────────────────────────────────────────────────────────

void screen_main_create(void)
{
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    build_top_bar(scr);
    build_display_area(scr);
    build_numpad(scr);
    build_preset_bar(scr);
    build_footer(scr);
}

// ─── Top bar ──────────────────────────────────────────────────────────────────

static void build_top_bar(lv_obj_t *parent)
{
    lv_obj_t *bar = lv_obj_create(parent);
    lv_obj_set_size(bar, SCREEN_W - 8, TOPBAR_H);
    lv_obj_align(bar, LV_ALIGN_TOP_MID, 0, 4);
    lv_obj_set_style_bg_color(bar, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(bar, 0, 0);
    lv_obj_set_style_pad_all(bar, 2, 0);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

    // Clock — left
    lbl_clock = lv_label_create(bar);
    lv_label_set_text(lbl_clock, "00:00:00");
    lv_obj_set_style_text_color(lbl_clock, lv_color_make(0, 255, 0), 0);
    lv_obj_align(lbl_clock, LV_ALIGN_LEFT_MID, 0, 0);

    // Station name — center
    lbl_station = lv_label_create(bar);
    lv_label_set_text(lbl_station, "---");
    lv_obj_set_style_text_color(lbl_station, lv_color_make(0, 170, 255), 0);
    lv_obj_align(lbl_station, LV_ALIGN_CENTER, 0, 0);

    // Country — right
    lbl_country = lv_label_create(bar);
    lv_label_set_text(lbl_country, "---");
    lv_obj_set_style_text_color(lbl_country, lv_color_make(255, 170, 0), 0);
    lv_obj_align(lbl_country, LV_ALIGN_RIGHT_MID, 0, 0);

    // Bottom border line
    lv_obj_t *line = lv_obj_create(parent);
    lv_obj_set_size(line, SCREEN_W - 8, 1);
    lv_obj_align(line, LV_ALIGN_TOP_MID, 0, TOPBAR_H + 4);
    lv_obj_set_style_bg_color(line, lv_color_make(40, 40, 40), 0);
    lv_obj_set_style_border_width(line, 0, 0);
}

// ─── Display area (frequency / spectrum toggle) ───────────────────────────────

static void build_display_area(lv_obj_t *parent)
{
    int y = TOPBAR_H + 8;

    // Outer container
    cont_display = lv_obj_create(parent);
    lv_obj_set_size(cont_display, DISPLAY_W, MAIN_H);
    lv_obj_set_pos(cont_display, 4, y);
    lv_obj_set_style_bg_color(cont_display, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(cont_display, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(cont_display, lv_color_make(60, 60, 60), 0);
    lv_obj_set_style_border_width(cont_display, 1, 0);
    lv_obj_set_style_pad_all(cont_display, 4, 0);
    lv_obj_clear_flag(cont_display, LV_OBJ_FLAG_SCROLLABLE);

    // Frequency label (7-seg style via monospace font)
    lbl_frequency = lv_label_create(cont_display);
    lv_label_set_text(lbl_frequency, "98.7");
    lv_obj_set_style_text_color(lbl_frequency, lv_color_make(0, 255, 0), 0);
    lv_obj_set_style_text_font(lbl_frequency, &lv_font_montserrat_28, 0);
    lv_obj_align(lbl_frequency, LV_ALIGN_CENTER, -8, -10);

    // Band label
    lbl_band = lv_label_create(cont_display);
    lv_label_set_text(lbl_band, "MHz  FM");
    lv_obj_set_style_text_color(lbl_band, lv_color_make(80, 80, 80), 0);
    lv_obj_align(lbl_band, LV_ALIGN_CENTER, -8, 18);

    // Signal bar — right side inside display
    bar_signal = lv_bar_create(cont_display);
    lv_obj_set_size(bar_signal, 10, MAIN_H - 12);
    lv_obj_align(bar_signal, LV_ALIGN_RIGHT_MID, -2, 0);
    lv_bar_set_range(bar_signal, 0, 100);
    lv_bar_set_value(bar_signal, 60, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(bar_signal, lv_color_make(30, 30, 30), LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar_signal, lv_color_make(0, 200, 0), LV_PART_INDICATOR);
}

// ─── Right numpad ─────────────────────────────────────────────────────────────

static const char *numpad_keys[] = {
    "1", "2", "3",
    "4", "5", "6",
    "7", "8", "9",
    ".", "0", "AM/FM",
    "<<", ">>", LV_SYMBOL_SETTINGS,
    "VOL-", "VOL+", NULL
};

static void build_numpad(lv_obj_t *parent)
{
    int x = DISPLAY_W + 8;
    int y = TOPBAR_H + 8;
    int w = SCREEN_W - x - 4;
    int h = MAIN_H;

    lv_obj_t *pad = lv_obj_create(parent);
    lv_obj_set_size(pad, w, h);
    lv_obj_set_pos(pad, x, y);
    lv_obj_set_style_bg_color(pad, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(pad, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(pad, 0, 0);
    lv_obj_set_style_pad_all(pad, 0, 0);
    lv_obj_set_style_pad_gap(pad, 2, 0);
    lv_obj_clear_flag(pad, LV_OBJ_FLAG_SCROLLABLE);

    int btn_w = (w - 4) / 3;
    // rows 1-4: numpad 3x4
    int numpad_rows = 4;
    int btn_h = (h - 2 - 22 - 22 - 6) / numpad_rows;

    for (int i = 0; i < 12; i++) {
        int col = i % 3;
        int row = i / 3;
        lv_obj_t *btn = lv_button_create(pad);
        lv_obj_set_size(btn, btn_w, btn_h);
        lv_obj_set_pos(btn, col * (btn_w + 2), row * (btn_h + 2));
        lv_obj_set_style_bg_color(btn, lv_color_make(20, 20, 20), 0);
        lv_obj_set_style_border_color(btn, lv_color_make(60, 60, 60), 0);
        lv_obj_set_style_border_width(btn, 1, 0);
        lv_obj_set_style_radius(btn, 3, 0);

        lv_obj_t *lbl = lv_label_create(btn);
        lv_label_set_text(lbl, numpad_keys[i]);
        lv_obj_center(lbl);

        // AM/FM button in red
        if (i == 11) {
            lv_obj_set_style_text_color(lbl, lv_color_make(255, 60, 60), 0);
            lv_obj_set_style_border_color(btn, lv_color_make(255, 60, 60), 0);
        } else {
            lv_obj_set_style_text_color(lbl, lv_color_make(170, 170, 170), 0);
        }
    }

    // Nav row: << >> settings
    int nav_y = numpad_rows * (btn_h + 2) + 2;
    const char *nav_keys[] = {"<<", ">>", LV_SYMBOL_SETTINGS};
    lv_color_t nav_colors[] = {
        lv_color_make(0, 170, 255),
        lv_color_make(0, 170, 255),
        lv_color_make(130, 130, 130)
    };
    for (int i = 0; i < 3; i++) {
        lv_obj_t *btn = lv_button_create(pad);
        lv_obj_set_size(btn, btn_w, 20);
        lv_obj_set_pos(btn, i * (btn_w + 2), nav_y);
        lv_obj_set_style_bg_color(btn, lv_color_make(20, 20, 20), 0);
        lv_obj_set_style_border_color(btn, lv_color_make(60, 60, 60), 0);
        lv_obj_set_style_border_width(btn, 1, 0);
        lv_obj_set_style_radius(btn, 3, 0);
        lv_obj_t *lbl = lv_label_create(btn);
        lv_label_set_text(lbl, nav_keys[i]);
        lv_obj_set_style_text_color(lbl, nav_colors[i], 0);
        lv_obj_center(lbl);
    }

    // Vol row: VOL- VOL+
    int vol_y = nav_y + 22;
    int vol_w = (w - 2) / 2;
    const char *vol_keys[] = {"VOL-", "VOL+"};
    for (int i = 0; i < 2; i++) {
        lv_obj_t *btn = lv_button_create(pad);
        lv_obj_set_size(btn, vol_w, 20);
        lv_obj_set_pos(btn, i * (vol_w + 2), vol_y);
        lv_obj_set_style_bg_color(btn, lv_color_make(20, 20, 20), 0);
        lv_obj_set_style_border_color(btn, lv_color_make(0, 180, 0), 0);
        lv_obj_set_style_border_width(btn, 1, 0);
        lv_obj_set_style_radius(btn, 3, 0);
        lv_obj_t *lbl = lv_label_create(btn);
        lv_label_set_text(lbl, vol_keys[i]);
        lv_obj_set_style_text_color(lbl, lv_color_make(0, 220, 0), 0);
        lv_obj_center(lbl);
    }
}

// ─── Preset bar ───────────────────────────────────────────────────────────────

static void build_preset_bar(lv_obj_t *parent)
{
    int y = SCREEN_H - FOOTER_H - PRESET_H - 2;
    lv_obj_t *bar = lv_obj_create(parent);
    lv_obj_set_size(bar, SCREEN_W - 8, PRESET_H);
    lv_obj_set_pos(bar, 4, y);
    lv_obj_set_style_bg_color(bar, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(bar, 0, 0);
    lv_obj_set_style_pad_all(bar, 0, 0);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

    // Label "P:"
    lv_obj_t *lbl_p = lv_label_create(bar);
    lv_label_set_text(lbl_p, "P:");
    lv_obj_set_style_text_color(lbl_p, lv_color_make(50, 50, 50), 0);
    lv_obj_align(lbl_p, LV_ALIGN_LEFT_MID, 0, 0);

    int total_w = SCREEN_W - 8 - 16;
    int btn_w = (total_w - 5 * 2) / 6;

    const char *labels[] = {"1", "2", "3", "4", "5", "SCAN"};
    for (int i = 0; i < 6; i++) {
        lv_obj_t *btn = lv_button_create(bar);
        lv_obj_set_size(btn, btn_w, PRESET_H - 2);
        lv_obj_set_pos(btn, 16 + i * (btn_w + 2), 0);
        lv_obj_set_style_bg_color(btn, lv_color_make(15, 15, 15), 0);
        lv_obj_set_style_radius(btn, 2, 0);

        bool is_scan = (i == 5);
        lv_color_t col = is_scan
            ? lv_color_make(80, 80, 80)
            : lv_color_make(0, 170, 255);
        lv_obj_set_style_border_color(btn, col, 0);
        lv_obj_set_style_border_width(btn, 1, 0);

        lv_obj_t *lbl = lv_label_create(btn);
        lv_label_set_text(lbl, labels[i]);
        lv_obj_set_style_text_color(lbl, col, 0);
        lv_obj_center(lbl);
    }
}

// ─── Footer ───────────────────────────────────────────────────────────────────

static void build_footer(lv_obj_t *parent)
{
    lv_obj_t *footer = lv_obj_create(parent);
    lv_obj_set_size(footer, SCREEN_W, FOOTER_H);
    lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(footer, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(footer, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(footer, 0, 0);
    lv_obj_set_style_border_side(footer, LV_BORDER_SIDE_TOP, 0);
    lv_obj_set_style_border_color(footer, lv_color_make(25, 25, 25), 0);
    lv_obj_clear_flag(footer, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl = lv_label_create(footer);
    lv_label_set_text(lbl, "damascoElectronics - CYD Radio v0.1");
    lv_obj_set_style_text_color(lbl, lv_color_make(35, 35, 35), 0);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_10, 0);
    lv_obj_center(lbl);
}

// ─── Public update functions ──────────────────────────────────────────────────

void screen_main_set_frequency(float freq)
{
    if (lbl_frequency == NULL) return;
    char buf[16];
    snprintf(buf, sizeof(buf), "%.1f", freq);
    lvgl_port_lock(0);
    lv_label_set_text(lbl_frequency, buf);
    lvgl_port_unlock();
}

void screen_main_set_station_name(const char *name)
{
    if (lbl_station == NULL) return;
    lvgl_port_lock(0);
    lv_label_set_text(lbl_station, name);
    lvgl_port_unlock();
}

void screen_main_set_signal(uint8_t level)
{
    if (bar_signal == NULL) return;
    lvgl_port_lock(0);
    lv_bar_set_value(bar_signal, level, LV_ANIM_ON);
    lvgl_port_unlock();
}

void screen_main_set_time(uint8_t h, uint8_t m, uint8_t s)
{
    if (lbl_clock == NULL) return;
    char buf[12];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, s);
    lvgl_port_lock(0);
    lv_label_set_text(lbl_clock, buf);
    lvgl_port_unlock();
}

void screen_main_set_country(const char *country)
{
    if (lbl_country == NULL) return;
    lvgl_port_lock(0);
    lv_label_set_text(lbl_country, country);
    lvgl_port_unlock();
}