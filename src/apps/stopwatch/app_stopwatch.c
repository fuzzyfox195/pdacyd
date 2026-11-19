#include "app_stopwatch.h"
#include "../../core/application_manager/application_manager.h"
#include "../../ui/assets/include_theme.h"
#include "../../ui/status_bar/status_bar.h"

#include <stdint.h>
#include <stdio.h>

enum StopwatchState {
    STOPWATCH_STATE_IDLE,
    STOPWATCH_STATE_RUNNING,
    STOPWATCH_STATE_PAUSED
};

#define STOPWATCH_UPDATE_PERIOD_MS 100
#define STOPWATCH_HOURS_DIGITS 3
#define STOPWATCH_MINUTES_DIGITS 2
#define STOPWATCH_SECONDS_DIGITS 2
#define STOPWATCH_COLON_GAP 4

static enum StopwatchState stopwatch_state;
static uint32_t stopwatch_elapsed_ms;
static uint32_t stopwatch_started_at;
static lv_obj_t *stopwatch_hours_label;
static lv_obj_t *stopwatch_minutes_label;
static lv_obj_t *stopwatch_seconds_label;
static lv_obj_t *stopwatch_control_label;
static lv_timer_t *stopwatch_update_timer;

static void stopwatch_update_display(void)
{
    if (stopwatch_hours_label == NULL || stopwatch_minutes_label == NULL ||
        stopwatch_seconds_label == NULL) return;

    uint32_t elapsed_ms = stopwatch_elapsed_ms;
    if (stopwatch_state == STOPWATCH_STATE_RUNNING) {
        elapsed_ms += lv_tick_get() - stopwatch_started_at;
    }

    uint32_t total_seconds = elapsed_ms / 1000U;
    uint32_t hours = total_seconds / 3600U;
    uint32_t minutes = (total_seconds % 3600U) / 60U;
    uint32_t seconds = total_seconds % 60U;

    char hours_text[STOPWATCH_HOURS_DIGITS + 1];
    char minutes_text[STOPWATCH_MINUTES_DIGITS + 1];
    char seconds_text[STOPWATCH_SECONDS_DIGITS + 1];
    snprintf(hours_text, sizeof(hours_text), "%03lu", (unsigned long)hours);
    snprintf(minutes_text, sizeof(minutes_text), "%02lu", (unsigned long)minutes);
    snprintf(seconds_text, sizeof(seconds_text), "%02lu", (unsigned long)seconds);
    lv_label_set_text(stopwatch_hours_label, hours_text);
    lv_label_set_text(stopwatch_minutes_label, minutes_text);
    lv_label_set_text(stopwatch_seconds_label, seconds_text);
}

static void stopwatch_update_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    stopwatch_update_display();
}

static bool stopwatch_init(void)
{
    stopwatch_state = STOPWATCH_STATE_IDLE;
    stopwatch_elapsed_ms = 0;
    stopwatch_started_at = 0;
    stopwatch_hours_label = NULL;
    stopwatch_minutes_label = NULL;
    stopwatch_seconds_label = NULL;
    stopwatch_control_label = NULL;
    stopwatch_update_timer = NULL;
    return true;
}

static void stopwatch_back_clicked(lv_event_t *event)
{
    (void)event;
    application_manager_close();
}

static void stopwatch_reset_clicked(lv_event_t *event)
{
    (void)event;
    stopwatch_elapsed_ms = 0;
    if (stopwatch_state == STOPWATCH_STATE_RUNNING) {
        stopwatch_started_at = lv_tick_get();
    } else {
        stopwatch_state = STOPWATCH_STATE_IDLE;
        if (stopwatch_control_label != NULL) {
            lv_label_set_text(stopwatch_control_label, "Start");
        }
    }
    stopwatch_update_display();
}

static void stopwatch_continue_pause_clicked(lv_event_t *event)
{
    (void)event;

    if (stopwatch_state == STOPWATCH_STATE_RUNNING) {
        stopwatch_elapsed_ms += lv_tick_get() - stopwatch_started_at;
        stopwatch_state = STOPWATCH_STATE_PAUSED;
        lv_label_set_text(stopwatch_control_label, "Continue");
    } else {
        stopwatch_started_at = lv_tick_get();
        stopwatch_state = STOPWATCH_STATE_RUNNING;
        lv_label_set_text(stopwatch_control_label, "Pause");
    }
    stopwatch_update_display();
}

static lv_obj_t *stopwatch_create_ui(void)
{
    lv_obj_t *screen = lv_obj_create(NULL);
    if (screen == NULL) return NULL;

    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(screen, COLOR_BACKGROUND, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);
    ui_status_bar_create(screen);

    const lv_coord_t digit_width = (lv_coord_t)lv_font_get_glyph_width(
        CONTENT_STOPWATCH_FONT, '0', '0');
    const lv_coord_t colon_width = (lv_coord_t)lv_font_get_glyph_width(
        CONTENT_STOPWATCH_FONT, ':', ':');
    const lv_coord_t hours_width = digit_width * STOPWATCH_HOURS_DIGITS;
    const lv_coord_t minutes_width = digit_width * STOPWATCH_MINUTES_DIGITS;
    const lv_coord_t seconds_width = digit_width * STOPWATCH_SECONDS_DIGITS;
    const lv_coord_t group_width = hours_width + minutes_width + seconds_width +
                                   colon_width * 2 + STOPWATCH_COLON_GAP * 4;

    lv_obj_t *time_row = lv_obj_create(screen);
    lv_obj_remove_style_all(time_row);
    lv_obj_clear_flag(time_row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(time_row, group_width, LV_SIZE_CONTENT);
    lv_obj_align(time_row, LV_ALIGN_TOP_MID, 0, 52);

    stopwatch_hours_label = lv_label_create(time_row);
    lv_label_set_text(stopwatch_hours_label, "000");
    lv_obj_set_width(stopwatch_hours_label, hours_width);
    lv_obj_set_style_text_font(stopwatch_hours_label, CONTENT_STOPWATCH_FONT, LV_PART_MAIN);
    lv_obj_set_style_text_color(stopwatch_hours_label, COLOR_MAIN_CONTENT, LV_PART_MAIN);
    lv_obj_set_style_text_align(stopwatch_hours_label, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
    lv_obj_align(stopwatch_hours_label, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_t *first_colon = lv_label_create(time_row);
    lv_label_set_text(first_colon, ":");
    lv_obj_set_width(first_colon, colon_width);
    lv_obj_set_style_text_font(first_colon, CONTENT_STOPWATCH_FONT, LV_PART_MAIN);
    lv_obj_set_style_text_color(first_colon, COLOR_MAIN_CONTENT, LV_PART_MAIN);
    lv_obj_set_style_text_align(first_colon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    const lv_coord_t first_colon_x = hours_width + STOPWATCH_COLON_GAP;
    lv_obj_align(first_colon, LV_ALIGN_TOP_LEFT, first_colon_x, 0);

    stopwatch_minutes_label = lv_label_create(time_row);
    lv_label_set_text(stopwatch_minutes_label, "00");
    lv_obj_set_width(stopwatch_minutes_label, minutes_width);
    lv_obj_set_style_text_font(stopwatch_minutes_label, CONTENT_STOPWATCH_FONT, LV_PART_MAIN);
    lv_obj_set_style_text_color(stopwatch_minutes_label, COLOR_MAIN_CONTENT, LV_PART_MAIN);
    lv_obj_set_style_text_align(stopwatch_minutes_label, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
    const lv_coord_t minutes_x = first_colon_x + colon_width + STOPWATCH_COLON_GAP;
    lv_obj_align(stopwatch_minutes_label, LV_ALIGN_TOP_LEFT, minutes_x, 0);

    lv_obj_t *second_colon = lv_label_create(time_row);
    lv_label_set_text(second_colon, ":");
    lv_obj_set_width(second_colon, colon_width);
    lv_obj_set_style_text_font(second_colon, CONTENT_STOPWATCH_FONT, LV_PART_MAIN);
    lv_obj_set_style_text_color(second_colon, COLOR_MAIN_CONTENT, LV_PART_MAIN);
    lv_obj_set_style_text_align(second_colon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    const lv_coord_t second_colon_x = minutes_x + minutes_width + STOPWATCH_COLON_GAP;
    lv_obj_align(second_colon, LV_ALIGN_TOP_LEFT, second_colon_x, 0);

    stopwatch_seconds_label = lv_label_create(time_row);
    lv_label_set_text(stopwatch_seconds_label, "00");
    lv_obj_set_width(stopwatch_seconds_label, seconds_width);
    lv_obj_set_style_text_font(stopwatch_seconds_label, CONTENT_STOPWATCH_FONT, LV_PART_MAIN);
    lv_obj_set_style_text_color(stopwatch_seconds_label, COLOR_MAIN_CONTENT, LV_PART_MAIN);
    lv_obj_set_style_text_align(stopwatch_seconds_label, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    const lv_coord_t seconds_x = second_colon_x + colon_width + STOPWATCH_COLON_GAP;
    lv_obj_align(stopwatch_seconds_label, LV_ALIGN_TOP_LEFT, seconds_x, 0);

    lv_obj_t *back_button = lv_btn_create(screen);
    lv_obj_set_size(back_button, 90, 20);
    lv_obj_align(back_button, LV_ALIGN_BOTTOM_LEFT, 12, -12);
    lv_obj_add_event_cb(back_button, stopwatch_back_clicked, LV_EVENT_CLICKED, NULL);

    lv_obj_t *control_button = lv_btn_create(screen);
    lv_obj_set_size(control_button, 140, 50);
    lv_obj_align(control_button, LV_ALIGN_LEFT_MID, 12, 30);
    lv_obj_add_event_cb(control_button, stopwatch_continue_pause_clicked, LV_EVENT_CLICKED, NULL);

    lv_obj_t *reset_button = lv_btn_create(screen);
    lv_obj_set_size(reset_button, 140, 50);
    lv_obj_align(reset_button, LV_ALIGN_RIGHT_MID, -12, 30);
    lv_obj_add_event_cb(reset_button, stopwatch_reset_clicked, LV_EVENT_CLICKED, NULL);

    lv_obj_t *back_label = lv_label_create(back_button);
    lv_label_set_text(back_label, "Back");
    lv_obj_set_style_text_font(back_label, FONT_SIZE_12, LV_PART_MAIN);
    lv_obj_center(back_label);

    stopwatch_control_label = lv_label_create(control_button);
    lv_label_set_text(stopwatch_control_label, "Start");
    lv_obj_set_style_text_font(stopwatch_control_label, CONTENT_STOPWATCH_CONTROL_FONT, LV_PART_MAIN);
    lv_obj_center(stopwatch_control_label);

    lv_obj_t *reset_label = lv_label_create(reset_button);
    lv_label_set_text(reset_label, "Reset");
    lv_obj_set_style_text_font(reset_label, CONTENT_STOPWATCH_CONTROL_FONT, LV_PART_MAIN);
    lv_obj_center(reset_label);

    stopwatch_update_timer = lv_timer_create(stopwatch_update_timer_cb,
                                             STOPWATCH_UPDATE_PERIOD_MS, NULL);
    if (stopwatch_update_timer == NULL) {
        stopwatch_hours_label = NULL;
        stopwatch_minutes_label = NULL;
        stopwatch_seconds_label = NULL;
        stopwatch_control_label = NULL;
        lv_obj_del(screen);
        return NULL;
    }

    return screen;
}

static void stopwatch_on_enter(void)
{
}

static void stopwatch_on_pause(void)
{
}

static void stopwatch_on_resume(void)
{
}

static void stopwatch_on_exit(void)
{
    if (stopwatch_update_timer != NULL) {
        lv_timer_del(stopwatch_update_timer);
        stopwatch_update_timer = NULL;
    }
}

static void stopwatch_deinit(void)
{
    if (stopwatch_update_timer != NULL) {
        lv_timer_del(stopwatch_update_timer);
        stopwatch_update_timer = NULL;
    }
    stopwatch_hours_label = NULL;
    stopwatch_minutes_label = NULL;
    stopwatch_seconds_label = NULL;
    stopwatch_control_label = NULL;
    stopwatch_elapsed_ms = 0;
    stopwatch_started_at = 0;
    stopwatch_state = STOPWATCH_STATE_IDLE;
}

const app_descriptor_t app_stopwatch = {
    .app_id = "stopwatch",
    .app_name = "Stopwatch",
    .init = stopwatch_init,
    .create_ui = stopwatch_create_ui,
    .on_enter = stopwatch_on_enter,
    .on_pause = stopwatch_on_pause,
    .on_resume = stopwatch_on_resume,
    .on_exit = stopwatch_on_exit,
    .deinit = stopwatch_deinit,
};

#ifdef LVGL_LIVE_PREVIEW
void lvgl_live_preview_init(void)
{
    stopwatch_init();
    lv_obj_t *screen = stopwatch_create_ui();
    if (screen != NULL) lv_screen_load(screen);
}
#endif
