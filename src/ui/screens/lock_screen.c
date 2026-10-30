#include "../assets/include_theme.h"
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

    lv_obj_t *user_name_label = lv_label_create(ui_lock_screen);
    lv_label_set_text(user_name_label, "Andrew");
    lv_obj_set_width(user_name_label, LV_SIZE_CONTENT);
    lv_obj_align(user_name_label, LV_ALIGN_BOTTOM_LEFT, 8, -14);
    lv_obj_set_style_text_align(user_name_label, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_style_text_font(user_name_label, HEADING_FONT, LV_PART_MAIN);

    lv_obj_t *unlock_guide_label = lv_label_create(ui_lock_screen);
    lv_label_set_text(unlock_guide_label, "Press\nto start ");
    lv_obj_set_width(unlock_guide_label, LV_SIZE_CONTENT);
    lv_obj_align(unlock_guide_label, LV_ALIGN_TOP_RIGHT, -48, 48);
    lv_obj_set_style_text_color(unlock_guide_label, COLOR_HINT_GRAY, LV_PART_MAIN);
    lv_obj_set_style_text_align(unlock_guide_label, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_style_text_font(unlock_guide_label, UNLOCK_GUIDE_FONT, LV_PART_MAIN);

    lv_obj_t *square_button = lv_obj_create(ui_lock_screen);
    lv_obj_set_size(square_button, 18, 18);
    lv_obj_align(square_button, LV_ALIGN_TOP_RIGHT, -46, 50);
    lv_obj_set_style_bg_color(square_button, COLOR_PRIMARY, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(square_button, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_color(square_button, COLOR_HINT_GRAY, LV_PART_MAIN);
    lv_obj_set_style_border_width(square_button, 3, LV_PART_MAIN);
    lv_obj_set_style_radius(square_button, RADIUS_DEFAULT, LV_PART_MAIN);
    lv_obj_clear_flag(square_button, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(square_button, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *button_direction = lv_label_create(ui_lock_screen);
    lv_obj_set_style_text_font(button_direction, &Icon_right_arrow_inside_circle, LV_PART_MAIN);
    lv_label_set_text(button_direction, "\xEF\x86\x8E");
    lv_obj_align(button_direction, LV_ALIGN_TOP_RIGHT, -48, 48);

}