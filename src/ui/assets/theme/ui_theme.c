#include "ui_theme.h"

static const ui_theme_palette_t light_theme = {
    .dark_mode = false,
    .main_content = 0x000000,
    .primary = 0x159150,
    .secondary = 0x293a2b,
    .hint_gray = 0x6E6E6E,
    .background = 0xFFFFFF,
};

static const ui_theme_palette_t dark_theme = {
    .dark_mode = true,
    .main_content = 0xF5F5F5,
    .primary = 0x0A84FF,
    .secondary = 0xBF5AF2,
    .hint_gray = 0xA1A1A6,
    .background = 0x121212,
};

static const ui_theme_palette_t sunset_theme = {
    .dark_mode = false,
    .main_content = 0x522D60,
    .primary = 0x522D60,
    .secondary = 0x69596d,
    .hint_gray = 0xC08467,
    .background = 0xFFD38B,
};

typedef struct {
    ui_theme_type_t type;
    const ui_theme_palette_t *palette;
} ui_theme_entry_t;

static const ui_theme_entry_t themes[] = {
    {UI_THEME_LIGHT, &light_theme},
    {UI_THEME_DARK, &dark_theme},
    {UI_THEME_SUNSET, &sunset_theme}
};

const ui_theme_palette_t *ui_theme_get_current_palette(void)
{
    for (size_t i = 0; i < sizeof(themes) / sizeof(themes[0]); ++i) {
        if (themes[i].type == ui_theme_current_type) {
            return themes[i].palette;
        }
    }

    return &light_theme;
}
