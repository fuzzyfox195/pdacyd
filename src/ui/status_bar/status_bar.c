#include "status_bar.h"
#include "../assets/include_theme.h"

#define STATUS_BAR_HEIGHT 24
#define STATUS_BAR_CONTENT_COLOR lv_color_hex(0x202124)

static lv_obj_t *battery_fill;
static lv_obj_t *battery_percent;
static int latest_adc_raw;

static lv_obj_t *make_rect(lv_obj_t *parent, lv_coord_t width, lv_coord_t height,
                           lv_color_t color, lv_coord_t radius)
{
    // Create a rectangle object with specified size, color, and radius
    // Will be used for battery fill, battery cap, and signal strength bars
    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, width, height);
    lv_obj_set_style_bg_color(obj, color, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(obj, radius, LV_PART_MAIN);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    return obj;
}

static void update_battery(int adc_raw)
{
    /* Set these to calibrated ADC readings for empty/full battery voltage. */
    const int adc_empty = 0;
    const int adc_full = 4095;
    int percent = (adc_raw - adc_empty) * 100 / (adc_full - adc_empty);
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    percent = ((percent + 2) / 5) * 5;

    if (battery_fill != NULL) {
        lv_obj_set_width(battery_fill, (lv_coord_t)(14 * percent / 100));
    }
    if (battery_percent != NULL) {
        static char text[5];
        lv_snprintf(text, sizeof(text), "%d%%", percent);
        lv_label_set_text(battery_percent, text);
    }
}

void ui_status_bar_set_battery_adc(int adc_raw)
{
    latest_adc_raw = adc_raw;
    update_battery(latest_adc_raw);
}

void ui_status_bar_create(lv_obj_t *parent)
{
    lv_obj_t *bar = lv_obj_create(parent);
    lv_obj_remove_style_all(bar);
    lv_obj_set_size(bar, 320, STATUS_BAR_HEIGHT);
    lv_obj_align(bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(bar, COLOR_BACKGROUND, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bar, LV_OPA_0, LV_PART_MAIN);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *time = lv_label_create(bar);
    lv_label_set_text(time, "12:30");
    lv_obj_set_style_text_color(time, STATUS_BAR_CONTENT_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_font(time, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_align(time, LV_ALIGN_LEFT_MID, 8, 4l);

    /* phone_connect placeholder: simple phone outline with a status dot */
    lv_obj_t *phone = lv_obj_create(bar);
    lv_obj_remove_style_all(phone);
    lv_obj_set_size(phone, 10, 16);
    lv_obj_align(phone, LV_ALIGN_RIGHT_MID, -99, 0);
    lv_obj_set_style_border_color(phone, STATUS_BAR_CONTENT_COLOR, LV_PART_MAIN);
    lv_obj_set_style_border_width(phone, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(phone, 2, LV_PART_MAIN);
    lv_obj_clear_flag(phone, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *phone_dot = make_rect(phone, 2, 2, STATUS_BAR_CONTENT_COLOR, LV_RADIUS_CIRCLE);
    lv_obj_align(phone_dot, LV_ALIGN_BOTTOM_MID, 0, -1);

    /* signal_strength placeholder: four ascending bars */
    const lv_coord_t heights[] = {5, 9, 14, 19};
    for (int i = 0; i < 4; ++i) {
        lv_obj_t *bar_segment = make_rect(bar, 4, heights[i], STATUS_BAR_CONTENT_COLOR, 0);
        lv_obj_align(bar_segment, LV_ALIGN_RIGHT_MID, -89 + i * 6, 10 - heights[i] / 2);
    }

    lv_obj_t *battery = lv_obj_create(bar);
    lv_obj_remove_style_all(battery);
    lv_obj_set_size(battery, 25, 15);
    lv_obj_align(battery, LV_ALIGN_RIGHT_MID, -45, 0);
    lv_obj_set_style_border_color(battery, STATUS_BAR_CONTENT_COLOR, LV_PART_MAIN);
    lv_obj_set_style_border_width(battery, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(battery, 3, LV_PART_MAIN);
    lv_obj_clear_flag(battery, LV_OBJ_FLAG_SCROLLABLE);
    // a little head at the right side of the batttery
    lv_obj_t *battery_cap = make_rect(bar, 3, 7, STATUS_BAR_CONTENT_COLOR, 1);
    lv_obj_align_to(battery_cap, battery, LV_ALIGN_OUT_RIGHT_MID, 1, 0);
    battery_fill = make_rect(battery, 14, 9, STATUS_BAR_CONTENT_COLOR, 1);
    lv_obj_align(battery_fill, LV_ALIGN_LEFT_MID, 2, 0);

    battery_percent = lv_label_create(bar);
    lv_obj_set_style_text_color(battery_percent, STATUS_BAR_CONTENT_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_font(battery_percent, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_align(battery_percent, LV_ALIGN_RIGHT_MID, -1, 0);

    // Add a separator line at the bottom of the status bar
    lv_obj_t *separator = make_rect(bar, 320, 1, lv_color_hex(0x777777), 0);
    lv_obj_align(separator, LV_ALIGN_BOTTOM_MID, 0, 0);
    update_battery(latest_adc_raw);
}
