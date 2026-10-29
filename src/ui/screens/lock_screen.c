#include "../main_ui.h"
#include "../assets/theme/ui_theme.h"
#include "lock_screen.h"

void ui_lock_screen_screen_init(void)
{
    ui_lock_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_lock_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_lock_screen, COLOR_BACKGROUND, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ui_lock_screen, LV_OPA_COVER, LV_PART_MAIN);

    lv_obj_t *time_label = lv_label_create(ui_lock_screen);
    lv_label_set_text(time_label, "19:30");
    lv_obj_set_width(time_label, LV_SIZE_CONTENT);
    lv_obj_align(time_label, LV_ALIGN_TOP_LEFT, 8, 4);
    lv_obj_set_style_text_align(time_label, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_style_text_font(time_label, CONTENT_FONT, LV_PART_MAIN);

    lv_obj_t *wellcome_label = lv_label_create(ui_lock_screen);
    lv_label_set_text(wellcome_label, "Good Afternoon,");
    lv_obj_set_width(wellcome_label, LV_SIZE_CONTENT);
    lv_obj_align(wellcome_label, LV_ALIGN_BOTTOM_LEFT, 8, -48);
    lv_obj_set_style_text_align(wellcome_label, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_style_text_font(wellcome_label, HEADING_FONT, LV_PART_MAIN);

    lv_obj_t *USER_NAME = lv_label_create(ui_lock_screen);
    lv_label_set_text(USER_NAME, "Andrew");
    lv_obj_set_width(USER_NAME, LV_SIZE_CONTENT);
    lv_obj_align(USER_NAME, LV_ALIGN_BOTTOM_LEFT, 8, -14);
    lv_obj_set_style_text_align(USER_NAME, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_style_text_font(USER_NAME, HEADING_FONT, LV_PART_MAIN);

    lv_obj_t *UNLOCK_GUIDE1 = lv_label_create(ui_lock_screen);
    lv_label_set_text(UNLOCK_GUIDE1, "Press ");
    lv_obj_set_width(UNLOCK_GUIDE1, LV_SIZE_CONTENT);
    lv_obj_align(UNLOCK_GUIDE1, LV_ALIGN_TOP_RIGHT, -64, 48);
    lv_obj_set_style_text_color(UNLOCK_GUIDE1, COLOR_HINT_GRAY, LV_PART_MAIN);
    lv_obj_set_style_text_align(UNLOCK_GUIDE1, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_style_text_font(UNLOCK_GUIDE1, CONTENT_FONT, LV_PART_MAIN);

    
    lv_obj_t *UNLOCK_GUIDE2 = lv_label_create(ui_lock_screen);
    lv_label_set_text(UNLOCK_GUIDE2, "to start");
    lv_obj_set_width(UNLOCK_GUIDE2, LV_SIZE_CONTENT);
    lv_obj_align(UNLOCK_GUIDE2, LV_ALIGN_TOP_RIGHT, -64, 70);
    lv_obj_set_style_text_color(UNLOCK_GUIDE2, COLOR_HINT_GRAY, LV_PART_MAIN);
    lv_obj_set_style_text_align(UNLOCK_GUIDE2, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_style_text_font(UNLOCK_GUIDE2, CONTENT_FONT, LV_PART_MAIN);
}