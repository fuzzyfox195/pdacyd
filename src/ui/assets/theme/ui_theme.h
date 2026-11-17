#pragma once

#ifndef UI_THEME_H
#define UI_THEME_H

#include "lvgl.h"

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
    uint32_t icon_voice_note;
    uint32_t background;
    uint32_t status_bar_content;
} ui_theme_palette_t;

extern ui_theme_type_t ui_theme_current_type;

const ui_theme_palette_t *ui_theme_get_current_palette(void);

#define COLOR_MAIN_CONTENT lv_color_hex(ui_theme_get_current_palette()->main_content)
#define COLOR_PRIMARY lv_color_hex(ui_theme_get_current_palette()->primary)
#define COLOR_SECONDARY lv_color_hex(ui_theme_get_current_palette()->secondary)
#define COLOR_HINT_GRAY lv_color_hex(ui_theme_get_current_palette()->hint_gray)
#define COLOR_ICON_VOICE_NOTE lv_color_hex(ui_theme_get_current_palette()->icon_voice_note)
#define COLOR_BACKGROUND lv_color_hex(ui_theme_get_current_palette()->background)
#define COLOR_STATUS_BAR_CONTENT lv_color_hex(ui_theme_get_current_palette()->status_bar_content)

#define RADIUS_DEFAULT  4
#define RADIUS_APP_TILE 8

#define HEADING_FONT &lv_font_montserrat_32
#define SUBHEADING_FONT &lv_font_montserrat_24
#define UNLOCK_GUIDE_FONT &lv_font_montserrat_18
#define CONTENT_FONT &lv_font_montserrat_14

#endif