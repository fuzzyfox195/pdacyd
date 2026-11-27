#include "status_bar.h"
#include "../assets/include_theme.h"

extern const lv_img_dsc_t icon_phone_connected;

#define STATUS_BAR_HEIGHT 24
#define STATUS_BAR_INSTANCE_LIMIT 8

static lv_obj_t *battery_fills[STATUS_BAR_INSTANCE_LIMIT];
static lv_obj_t *battery_labels[STATUS_BAR_INSTANCE_LIMIT];
static lv_obj_t *time_labels[STATUS_BAR_INSTANCE_LIMIT];
static lv_obj_t *wifi_segments[STATUS_BAR_INSTANCE_LIMIT][4];
static lv_obj_t *wifi_error_labels[STATUS_BAR_INSTANCE_LIMIT];
static uint8_t latest_battery_percent;
static char latest_battery_text[5] = "0%";
static char latest_time_text[6] = "00:00";
static bool latest_wifi_connected;
static bool latest_wifi_error;
static uint8_t latest_wifi_signal_level;

static void battery_fill_deleted(lv_event_t *event)
{
    lv_obj_t *deleted_fill = lv_event_get_target(event);
    for (size_t i = 0; i < STATUS_BAR_INSTANCE_LIMIT; ++i) {
        if (battery_fills[i] == deleted_fill) {
            battery_fills[i] = NULL;
            return;
        }
    }
}

static void battery_label_deleted(lv_event_t *event)
{
    lv_obj_t *deleted_label = lv_event_get_target(event);
    for (size_t i = 0; i < STATUS_BAR_INSTANCE_LIMIT; ++i) {
        if (battery_labels[i] == deleted_label) {
            battery_labels[i] = NULL;
            return;
        }
    }
}

static void time_label_deleted(lv_event_t *event)
{
    lv_obj_t *deleted_label = lv_event_get_target(event);
    for (size_t i = 0; i < STATUS_BAR_INSTANCE_LIMIT; ++i) {
        if (time_labels[i] == deleted_label) {
            time_labels[i] = NULL;
            return;
        }
    }
}

static void wifi_object_deleted(lv_event_t *event)
{
    lv_obj_t *deleted_object = lv_event_get_target(event);
    for (size_t i = 0; i < STATUS_BAR_INSTANCE_LIMIT; ++i) {
        if (wifi_error_labels[i] == deleted_object) {
            wifi_error_labels[i] = NULL;
            return;
        }
        for (size_t segment = 0; segment < 4; ++segment) {
            if (wifi_segments[i][segment] == deleted_object) {
                wifi_segments[i][segment] = NULL;
                return;
            }
        }
    }
}

static int reserve_wifi_slot(void)
{
    for (size_t i = 0; i < STATUS_BAR_INSTANCE_LIMIT; ++i) {
        bool slot_free = wifi_error_labels[i] == NULL;
        for (size_t segment = 0; segment < 4; ++segment) {
            if (wifi_segments[i][segment] != NULL) slot_free = false;
        }
        if (slot_free) return (int)i;
    }
    return -1;
}

static void update_wifi_slot(size_t slot)
{
    for (size_t segment = 0; segment < 4; ++segment) {
        lv_obj_t *bar = wifi_segments[slot][segment];
        if (bar == NULL) continue;
        if (latest_wifi_connected && segment < latest_wifi_signal_level) {
            lv_obj_clear_flag(bar, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(bar, LV_OBJ_FLAG_HIDDEN);
        }
    }
    if (wifi_error_labels[slot] != NULL) {
        if (latest_wifi_error) {
            lv_obj_clear_flag(wifi_error_labels[slot], LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(wifi_error_labels[slot], LV_OBJ_FLAG_HIDDEN);
        }
    }
}

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

static void update_battery(uint8_t percent)
{
    for (size_t i = 0; i < STATUS_BAR_INSTANCE_LIMIT; ++i) {
        if (battery_fills[i] != NULL) {
            lv_obj_set_width(battery_fills[i], (lv_coord_t)(14 * percent / 100));
        }
        if (battery_labels[i] != NULL) {
            lv_label_set_text(battery_labels[i], latest_battery_text);
        }
    }
}

void ui_status_bar_set_battery_percent(uint8_t percent)
{
    latest_battery_percent = percent > 100 ? 100 : percent;
    lv_snprintf(latest_battery_text, sizeof(latest_battery_text), "%u%%",
                (unsigned)latest_battery_percent);
    update_battery(latest_battery_percent);
}

void ui_status_bar_set_time(uint8_t hour, uint8_t minute, bool valid)
{
    if (valid) {
        lv_snprintf(latest_time_text, sizeof(latest_time_text), "%02u:%02u",
                    (unsigned)hour, (unsigned)minute);
    } else {
        lv_snprintf(latest_time_text, sizeof(latest_time_text), "00:00");
    }
    for (size_t i = 0; i < STATUS_BAR_INSTANCE_LIMIT; ++i) {
        if (time_labels[i] != NULL) lv_label_set_text(time_labels[i], latest_time_text);
    }
}

void ui_status_bar_set_wifi_status(bool connected, bool error, uint8_t signal_level)
{
    latest_wifi_connected = connected;
    latest_wifi_error = error && !connected;
    latest_wifi_signal_level = signal_level > 4 ? 4 : signal_level;
    for (size_t i = 0; i < STATUS_BAR_INSTANCE_LIMIT; ++i) {
        update_wifi_slot(i);
    }
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
    // New app screens inherit the last clock value instead of waiting for the
    // next minute/second update from the system snapshot.
    lv_label_set_text(time, latest_time_text);
    lv_obj_set_style_text_color(time, COLOR_MAIN_CONTENT, LV_PART_MAIN);
    lv_obj_set_style_text_font(time, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_align(time, LV_ALIGN_LEFT_MID, 8, 2);
    bool time_tracked = false;
    for (size_t i = 0; i < STATUS_BAR_INSTANCE_LIMIT; ++i) {
        if (time_labels[i] == NULL) {
            time_labels[i] = time;
            time_tracked = true;
            break;
        }
    }
    if (time_tracked) lv_obj_add_event_cb(time, time_label_deleted, LV_EVENT_DELETE, NULL);

    const lv_coord_t icon_gap = 8;
    const lv_coord_t battery_width = 25;
    const lv_coord_t signal_width = 24;
    const lv_coord_t battery_right_offset = -64;
    const lv_coord_t signal_right_offset = battery_right_offset - battery_width - icon_gap;
    const lv_coord_t phone_right_offset = signal_right_offset - signal_width - icon_gap;
    const int wifi_slot = reserve_wifi_slot();

    lv_obj_t *battery = lv_obj_create(bar);
    lv_obj_remove_style_all(battery);
    lv_obj_set_size(battery, battery_width, 15);
    lv_obj_align(battery, LV_ALIGN_RIGHT_MID, battery_right_offset, 2);
    lv_obj_set_style_border_color(battery, COLOR_MAIN_CONTENT, LV_PART_MAIN);
    lv_obj_set_style_border_width(battery, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(battery, 3, LV_PART_MAIN);
    lv_obj_clear_flag(battery, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *battery_cap = make_rect(bar, 3, 7, COLOR_MAIN_CONTENT, 1);
    lv_obj_align_to(battery_cap, battery, LV_ALIGN_OUT_RIGHT_MID, 1, 0);
    lv_obj_t *battery_fill = make_rect(battery, 14, 9, COLOR_MAIN_CONTENT, 1);
    lv_obj_align(battery_fill, LV_ALIGN_LEFT_MID, 1, 0);
    bool tracked = false;
    for (size_t i = 0; i < STATUS_BAR_INSTANCE_LIMIT; ++i) {
        if (battery_fills[i] == NULL) {
            battery_fills[i] = battery_fill;
            tracked = true;
            break;
        }
    }
    if (tracked) {
        lv_obj_add_event_cb(battery_fill, battery_fill_deleted, LV_EVENT_DELETE, NULL);
    }

    const char battery_max_text[] = "100%";
    const lv_coord_t battery_label_width = lv_txt_get_width(
        battery_max_text, sizeof(battery_max_text) - 1, &lv_font_montserrat_18,
        0, LV_TEXT_FLAG_NONE);
    lv_obj_t *battery_label = lv_label_create(bar);
    lv_label_set_text(battery_label, latest_battery_text);
    lv_obj_set_width(battery_label, battery_label_width);
    lv_obj_set_style_text_font(battery_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_set_style_text_color(battery_label, COLOR_MAIN_CONTENT, LV_PART_MAIN);
    lv_obj_set_style_text_align(battery_label, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
    lv_obj_align(battery_label, LV_ALIGN_RIGHT_MID, -8, 2);
    tracked = false;
    for (size_t i = 0; i < STATUS_BAR_INSTANCE_LIMIT; ++i) {
        if (battery_labels[i] == NULL) {
            battery_labels[i] = battery_label;
            tracked = true;
            break;
        }
    }
    if (tracked) {
        lv_obj_add_event_cb(battery_label, battery_label_deleted, LV_EVENT_DELETE, NULL);
    }

    update_battery(latest_battery_percent);

    lv_obj_t *signal_base = make_rect(bar, signal_width, 1, COLOR_BACKGROUND, 0);
    lv_obj_align(signal_base, LV_ALIGN_BOTTOM_RIGHT, signal_right_offset, -2);
    const lv_coord_t heights[] = {7, 10, 13, 15};
    for (int i = 0; i < 4; ++i) {
        lv_obj_t *bar_segment = make_rect(bar, 4, heights[i], COLOR_MAIN_CONTENT, 1);
        lv_obj_align_to(bar_segment, signal_base, LV_ALIGN_BOTTOM_LEFT, 1 + i * 6, 0);
        lv_obj_add_flag(bar_segment, LV_OBJ_FLAG_HIDDEN);
        if (wifi_slot >= 0) {
            wifi_segments[wifi_slot][i] = bar_segment;
            lv_obj_add_event_cb(bar_segment, wifi_object_deleted, LV_EVENT_DELETE, NULL);
        }
    }

    lv_obj_t *wifi_error = lv_label_create(bar);
    lv_label_set_text(wifi_error, "!");
    lv_obj_set_style_text_font(wifi_error, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(wifi_error, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);
    lv_obj_align(wifi_error, LV_ALIGN_BOTTOM_RIGHT, signal_right_offset + 5, 1);
    lv_obj_add_flag(wifi_error, LV_OBJ_FLAG_HIDDEN);
    if (wifi_slot >= 0) {
        wifi_error_labels[wifi_slot] = wifi_error;
        lv_obj_add_event_cb(wifi_error, wifi_object_deleted, LV_EVENT_DELETE, NULL);
    }
    if (wifi_slot >= 0) update_wifi_slot((size_t)wifi_slot);

    lv_obj_t *phone = lv_img_create(bar);
    lv_img_set_src(phone, &icon_phone_connected);
    lv_img_set_zoom(phone, 205); /* 80% of the original 24x24 asset */
    lv_obj_set_style_img_recolor(phone,COLOR_MAIN_CONTENT, LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(phone, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_align(phone, LV_ALIGN_RIGHT_MID, phone_right_offset, 2);
}
