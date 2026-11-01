#include "main_ui.h"
#include "include_screen.h"

lv_obj_t * ui_lock_screen;
lv_obj_t * ui_start_screen;

void ui_init(void)
{
    lv_disp_t * dispp = lv_disp_get_default();
    lv_theme_t * theme = lv_theme_default_init(dispp, lv_color_hex(0x2196F3), lv_color_hex(0xEF6C00), false, &lv_font_montserrat_14);
    lv_disp_set_theme(dispp, theme);

    ui_lock_screen_screen_init();
    lv_disp_load_scr(ui_lock_screen);
}