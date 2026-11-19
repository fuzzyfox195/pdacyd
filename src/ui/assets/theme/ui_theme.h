#pragma once

#ifndef UI_THEME_H
#define UI_THEME_H

#include "lvgl.h"

LV_FONT_DECLARE(Google_san_code_bold_48px);

typedef enum {
    UI_THEME_LIGHT,
    UI_THEME_DARK,
    UI_THEME_SUNSET
} ui_theme_type_t;

typedef struct {
    bool dark_mode;
    uint32_t main_content;
    uint32_t primary;
    uint32_t secondary;
    uint32_t hint_gray;
    uint32_t background;
} ui_theme_palette_t;

extern ui_theme_type_t ui_theme_current_type;

const ui_theme_palette_t *ui_theme_get_current_palette(void);

#define COLOR_MAIN_CONTENT lv_color_hex(ui_theme_get_current_palette()->main_content)
#define COLOR_PRIMARY lv_color_hex(ui_theme_get_current_palette()->primary)
#define COLOR_SECONDARY lv_color_hex(ui_theme_get_current_palette()->secondary)
#define COLOR_HINT_GRAY lv_color_hex(ui_theme_get_current_palette()->hint_gray)
#define COLOR_BACKGROUND lv_color_hex(ui_theme_get_current_palette()->background)
#define RADIUS_DEFAULT  4
#define RADIUS_APP_TILE 8

#define HEADING_FONT &lv_font_montserrat_32
#define SUBHEADING_FONT &lv_font_montserrat_24
#define UNLOCK_GUIDE_FONT &lv_font_montserrat_18
#define CONTENT_FONT &lv_font_montserrat_14
#define CONTENT_STOPWATCH_FONT &Google_san_code_bold_48px
#define CONTENT_STOPWATCH_CONTROL_FONT &lv_font_montserrat_24


// For testing font size
#define FONT_SIZE_12 &lv_font_montserrat_12
#define FONT_SIZE_14 &lv_font_montserrat_14
#define FONT_SIZE_16 &lv_font_montserrat_16
#define FONT_SIZE_18 &lv_font_montserrat_18
#define FONT_SIZE_20 &lv_font_montserrat_20
#define FONT_SIZE_22 &lv_font_montserrat_22

#endif
