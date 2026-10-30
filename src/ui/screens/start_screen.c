#include "../assets/include_theme.h"
#include "start_screen.h"

void ui_start_screen_screen_init(void)
{
    ui_start_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_start_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_start_screen, COLOR_BACKGROUND, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ui_start_screen, LV_OPA_COVER, LV_PART_MAIN);

}