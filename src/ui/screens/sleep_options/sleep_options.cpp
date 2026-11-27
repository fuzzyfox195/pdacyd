#include "sleep_options.h"

#include "../../../core/display/display_manager.h"
#include "../../../core/power/power_manager.h"
#include "../../../core/screen_manager/screen_manager.h"
#include "../../assets/include_theme.h"
#include "../../status_bar/status_bar.h"

namespace {
lv_obj_t *options_screen;
lv_obj_t *status_label;

void lockAfterSleepAsync(void *user_data);

void closeClicked(lv_event_t *event)
{
    (void)event;
    sleep_options_close();
}

void modeClicked(lv_event_t *event)
{
    const power_sleep_mode_t mode = static_cast<power_sleep_mode_t>(
        reinterpret_cast<uintptr_t>(lv_event_get_user_data(event)));
    const bool success = power_manager_request_sleep(mode);
    if (status_label == nullptr) return;
    if (success && (mode == POWER_MODEM_SLEEP || mode == POWER_LIGHT_SLEEP)) {
        sleep_options_lock_after_sleep(mode == POWER_MODEM_SLEEP);
        return;
    }
    if (!success) {
        lv_label_set_text(status_label, "Power mode failed. Check device state.");
    } else if (mode == POWER_MODEM_SLEEP) {
        lv_label_set_text(status_label, "Wi-Fi modem sleep enabled.");
    } else if (mode == POWER_LIGHT_SLEEP) {
        lv_label_set_text(status_label, "Awake. Sleep button woke the PDA.");
    }
}

void createButton(lv_obj_t *parent, const char *text, lv_coord_t y,
                  lv_event_cb_t callback, void *user_data)
{
    lv_obj_t *button = lv_btn_create(parent);
    lv_obj_set_size(button, 210, 34);
    lv_obj_align(button, LV_ALIGN_TOP_MID, 0, y);
    lv_obj_add_event_cb(button, callback, LV_EVENT_CLICKED, user_data);
    lv_obj_t *label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_center(label);
}

void lockAfterSleepAsync(void *user_data)
{
    const bool sleep_display = reinterpret_cast<uintptr_t>(user_data) != 0;
    options_screen = nullptr;
    status_label = nullptr;
    screen_manager_show_sleep_lock();
    if (sleep_display) display_manager_sleep();
}
} // namespace

extern "C" bool sleep_options_show(void)
{
    if (screen_manager_has_modal()) return false;
    options_screen = lv_obj_create(NULL);
    if (options_screen == nullptr) return false;
    lv_obj_clear_flag(options_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(options_screen, COLOR_BACKGROUND, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(options_screen, LV_OPA_COVER, LV_PART_MAIN);
    ui_status_bar_create(options_screen);

    lv_obj_t *title = lv_label_create(options_screen);
    lv_label_set_text(title, "Sleep options");
    lv_obj_set_style_text_font(title, HEADING_FONT, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, COLOR_MAIN_CONTENT, LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 38);

    createButton(options_screen, "Modem sleep", 66, modeClicked,
                 reinterpret_cast<void *>(static_cast<uintptr_t>(POWER_MODEM_SLEEP)));
    createButton(options_screen, "Light sleep", 104, modeClicked,
                 reinterpret_cast<void *>(static_cast<uintptr_t>(POWER_LIGHT_SLEEP)));
    createButton(options_screen, "Deep sleep", 142, modeClicked,
                 reinterpret_cast<void *>(static_cast<uintptr_t>(POWER_DEEP_SLEEP)));

    status_label = lv_label_create(options_screen);
    lv_label_set_text(status_label, "Hold Sleep for 2.5 s for immediate Deep sleep.");
    lv_obj_set_width(status_label, 300);
    lv_obj_set_style_text_align(status_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_color(status_label, COLOR_HINT_GRAY, LV_PART_MAIN);
    lv_obj_set_style_text_font(status_label, CONTENT_FONT, LV_PART_MAIN);
    lv_obj_align(status_label, LV_ALIGN_TOP_MID, 0, 179);

    createButton(options_screen, "Cancel", 202, closeClicked, nullptr);
    if (!screen_manager_push_modal(options_screen)) {
        lv_obj_del(options_screen);
        options_screen = nullptr;
        status_label = nullptr;
        return false;
    }
    return true;
}

extern "C" void sleep_options_close(void)
{
    if (!screen_manager_has_modal()) return;
    (void)screen_manager_pop_modal();
    options_screen = nullptr;
    status_label = nullptr;
}

extern "C" void sleep_options_lock_after_sleep(bool sleep_display)
{
    lv_async_call(lockAfterSleepAsync,
                  reinterpret_cast<void *>(static_cast<uintptr_t>(sleep_display)));
}
