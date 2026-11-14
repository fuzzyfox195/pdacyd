#include "start_screen.h"
#include "../../main_ui.h"
#include "../../status_bar/status_bar.h"
#include "../../assets/include_theme.h"

static lv_obj_t *draw_function_big(lv_obj_t *parent, const char *appName,
                                   const char *insideContent, int x, int y,
                                   lv_color_t color)
{
    // This tile have fixed size: 100x100 px
    lv_obj_t *tile = lv_obj_create(parent);
    lv_obj_remove_style_all(tile);
    lv_obj_set_size(tile, 100, 100);
    lv_obj_set_pos(tile, x, y);
    lv_obj_set_style_bg_color(tile, COLOR_BACKGROUND, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(tile, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(tile, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(tile, color, LV_PART_MAIN);
    lv_obj_set_style_radius(tile, RADIUS_APP_TILE, LV_PART_MAIN);
    // App name and title
    lv_obj_t *app_name = lv_label_create(tile);
    lv_label_set_text(app_name, appName);
    lv_obj_set_style_text_color(app_name, color, LV_PART_MAIN);
    lv_obj_set_style_text_font(app_name, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(app_name, LV_ALIGN_BOTTOM_LEFT, 4, -2);

    
    return tile;
}


void ui_start_screen_screen_init(void)
{
    ui_start_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_start_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_start_screen, COLOR_BACKGROUND, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ui_start_screen, LV_OPA_COVER, LV_PART_MAIN);
    ui_status_bar_create(ui_start_screen);

    lv_obj_t *app_voice_note = draw_function_big(ui_start_screen, "Voice Note", "12 Notes", 8, 30, COLOR_ICON_VOICE_NOTE);

}

#ifdef LVGL_LIVE_PREVIEW
// gives the live preview tool a way to initialize your UI
void lvgl_live_preview_init(void) {
    ui_init();
}
#endif
