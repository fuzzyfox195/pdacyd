#include "main_ui.h"
#include "include_screen.h"
#include "assets/include_theme.h"

lv_obj_t * ui_lock_screen;
lv_obj_t * ui_start_screen;
ui_theme_type_t ui_theme_current_type = UI_THEME_LIGHT;


void ui_init(void)
{
    lv_disp_t * dispp = lv_disp_get_default();
    lv_theme_t * theme = lv_theme_default_init(
        dispp, COLOR_PRIMARY, COLOR_SECONDARY,
        ui_theme_get_current_palette()->dark_mode, &lv_font_montserrat_14);
    lv_disp_set_theme(dispp, theme);

    ui_lock_screen_screen_init();
    ui_start_screen_screen_init();
    lv_disp_load_scr(ui_start_screen);
}