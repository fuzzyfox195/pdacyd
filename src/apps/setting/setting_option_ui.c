#include "setting_option_ui.h"
#include "../../ui/assets/include_theme.h"

void setting_option_ui_create(lv_obj_t *parent, const char *title,
                              const char *description)
{
    lv_obj_t *heading = lv_label_create(parent);
    lv_label_set_text(heading, title);
    lv_obj_set_style_text_color(heading, COLOR_MAIN_CONTENT, LV_PART_MAIN);
    lv_obj_set_style_text_font(heading, &lv_font_montserrat_22, LV_PART_MAIN);
    lv_obj_align(heading, LV_ALIGN_TOP_LEFT, 12, 12);

    lv_obj_t *placeholder = lv_label_create(parent);
    lv_label_set_text(placeholder, description);
    lv_obj_set_width(placeholder, lv_pct(100));
    lv_label_set_long_mode(placeholder, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(placeholder, COLOR_HINT_GRAY, LV_PART_MAIN);
    lv_obj_set_style_text_font(placeholder, CONTENT_FONT, LV_PART_MAIN);
    lv_obj_align_to(placeholder, heading, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
}
