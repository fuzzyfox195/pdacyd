#include "app_system_log.h"
#include "system_log_storage.h"
#include "../../core/application_manager/application_manager.h"
#include "../../ui/assets/include_theme.h"
#include "../../ui/status_bar/status_bar.h"

extern const lv_img_dsc_t system_log_icon;

#define SYSTEM_LOG_BUFFER_SIZE (12 * 1024)

static lv_obj_t *log_screen;
static lv_obj_t *log_panel;
static lv_obj_t *log_text;
static lv_obj_t *loading_spinner;
static lv_obj_t *loading_label;
static lv_timer_t *load_timer;
static char log_buffer[SYSTEM_LOG_BUFFER_SIZE + 1];

static bool system_log_init(void)
{
    return true;
}

static void show_loading(void)
{
    if (loading_spinner != NULL) lv_obj_clear_flag(loading_spinner, LV_OBJ_FLAG_HIDDEN);
    if (loading_label != NULL) lv_obj_clear_flag(loading_label, LV_OBJ_FLAG_HIDDEN);
    if (log_text != NULL) lv_obj_add_flag(log_text, LV_OBJ_FLAG_HIDDEN);
}

static void system_log_load_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    load_timer = NULL;
    if (log_screen == NULL || log_text == NULL) return;

    if (app_system_log_read_tail(log_buffer, sizeof(log_buffer)) && log_buffer[0] != '\0') {
        lv_label_set_text(log_text, log_buffer);
    } else {
        lv_label_set_text(log_text, "No log entries found on the SD card.");
    }

    if (loading_spinner != NULL) lv_obj_add_flag(loading_spinner, LV_OBJ_FLAG_HIDDEN);
    if (loading_label != NULL) lv_obj_add_flag(loading_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(log_text, LV_OBJ_FLAG_HIDDEN);
}

static void start_log_load(void)
{
    if (log_screen == NULL) return;
    if (load_timer != NULL) {
        lv_timer_del(load_timer);
        load_timer = NULL;
    }
    show_loading();
    load_timer = lv_timer_create(system_log_load_timer_cb, 180, NULL);
    if (load_timer != NULL) lv_timer_set_repeat_count(load_timer, 1);
}

static void system_log_back_clicked(lv_event_t *event)
{
    (void)event;
    application_manager_close();
}

static void system_log_refresh_clicked(lv_event_t *event)
{
    (void)event;
    start_log_load();
}

static lv_obj_t *system_log_create_ui(void)
{
    log_screen = lv_obj_create(NULL);
    if (log_screen == NULL) return NULL;

    lv_obj_clear_flag(log_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(log_screen, COLOR_BACKGROUND, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(log_screen, LV_OPA_COVER, LV_PART_MAIN);
    ui_status_bar_create(log_screen);

    const lv_coord_t screen_height = lv_disp_get_ver_res(lv_disp_get_default());
    log_panel = lv_obj_create(log_screen);
    lv_obj_set_size(log_panel, 310, screen_height - 100);
    lv_obj_align(log_panel, LV_ALIGN_TOP_MID, 0, 40);
    lv_obj_set_style_bg_color(log_panel, COLOR_BACKGROUND, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(log_panel, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(log_panel, COLOR_HINT_GRAY, LV_PART_MAIN);
    lv_obj_set_style_border_width(log_panel, 1, LV_PART_MAIN);
    lv_obj_set_scroll_dir(log_panel, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(log_panel, LV_SCROLLBAR_MODE_AUTO);

    log_text = lv_label_create(log_panel);
    lv_label_set_text(log_text, "");
    lv_label_set_long_mode(log_text, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(log_text, 278);
    lv_obj_set_style_text_font(log_text, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_style_text_color(log_text, COLOR_MAIN_CONTENT, LV_PART_MAIN);
    lv_obj_align(log_text, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_add_flag(log_text, LV_OBJ_FLAG_HIDDEN);

    loading_spinner = lv_spinner_create(log_panel, 1000, 60);
    lv_obj_set_size(loading_spinner, 38, 38);
    lv_obj_align(loading_spinner, LV_ALIGN_CENTER, 0, -14);

    loading_label = lv_label_create(log_panel);
    lv_label_set_text(loading_label, "Loading logs from SD...");
    lv_obj_set_style_text_color(loading_label, COLOR_HINT_GRAY, LV_PART_MAIN);
    lv_obj_align(loading_label, LV_ALIGN_CENTER, 0, 24);

    lv_obj_t *back_button = lv_btn_create(log_screen);
    lv_obj_set_size(back_button, 90, 38);
    lv_obj_align(back_button, LV_ALIGN_BOTTOM_LEFT, 12, -8);
    lv_obj_add_event_cb(back_button, system_log_back_clicked, LV_EVENT_CLICKED, NULL);
    lv_obj_t *back_label = lv_label_create(back_button);
    lv_label_set_text(back_label, "Back");
    lv_obj_center(back_label);

    lv_obj_t *refresh_button = lv_btn_create(log_screen);
    lv_obj_set_size(refresh_button, 90, 38);
    lv_obj_align(refresh_button, LV_ALIGN_BOTTOM_RIGHT, -12, -8);
    lv_obj_add_event_cb(refresh_button, system_log_refresh_clicked, LV_EVENT_CLICKED, NULL);
    lv_obj_t *refresh_label = lv_label_create(refresh_button);
    lv_label_set_text(refresh_label, "Refresh");
    lv_obj_center(refresh_label);

    return log_screen;
}

static void system_log_on_enter(void)
{
    start_log_load();
}

static void system_log_on_pause(void) {}
static void system_log_on_resume(void) {}

static void system_log_on_exit(void)
{
    if (load_timer != NULL) {
        lv_timer_del(load_timer);
        load_timer = NULL;
    }
    log_screen = NULL;
    log_panel = NULL;
    log_text = NULL;
    loading_spinner = NULL;
    loading_label = NULL;
}

static void system_log_deinit(void)
{
    system_log_on_exit();
}

const app_descriptor_t app_system_log = {
    .app_id = "system_log",
    .app_name = "System Log",
    .start_tile = {
        .icon = &system_log_icon,
        .color_hex = 0xF28A12,
        .size = APP_TILE_SIZE_SMALL,
        .x = 8,
        .y = 324,
        .title = NULL,
        .content = NULL,
    },
    .init = system_log_init,
    .create_ui = system_log_create_ui,
    .on_enter = system_log_on_enter,
    .on_pause = system_log_on_pause,
    .on_resume = system_log_on_resume,
    .on_exit = system_log_on_exit,
    .deinit = system_log_deinit,
};