#include "app_timer.h"
#include "../../core/application_manager/application_manager.h"
#include "../../ui/assets/include_theme.h"
#include "../../ui/status_bar/status_bar.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

extern const lv_img_dsc_t timer_icon;

enum TimerState {
    TIMER_STATE_IDLE, // no time set, no timer running
    TIMER_STATE_SETTED, // time set, but timer not running
    TIMER_STATE_RUNNING, // timer running
    TIMER_STATE_PAUSED // time set, timer paused
};

#define TIMER_UPDATE_PERIOD_MS 100
#define TIMER_HOURS_DIGITS 2
#define TIMER_MINUTES_DIGITS 2
#define TIMER_SECONDS_DIGITS 2
#define TIMER_COLON_GAP 4
#define TIMER_ADJUST_BUTTON_WIDTH 52
#define TIMER_ADJUST_BUTTON_HEIGHT 30

typedef enum {
    TIMER_UNIT_HOURS,
    TIMER_UNIT_MINUTES,
    TIMER_UNIT_SECONDS,
    TIMER_UNIT_COUNT
} timer_unit_t;

typedef struct {
    timer_unit_t unit;
    int8_t delta;
} timer_adjust_data_t;

static enum TimerState timer_state;
static uint32_t timer_elapsed_ms;
static uint32_t timer_started_at;
static uint32_t timer_setted_ms;
static lv_obj_t *timer_hours_label;
static lv_obj_t *timer_minutes_label;
static lv_obj_t *timer_seconds_label;
static lv_obj_t *timer_control_label;
static lv_timer_t *timer_update_timer;


static void timer_update_display(void)
{
    if (timer_hours_label == NULL || timer_minutes_label == NULL ||
        timer_seconds_label == NULL) return;

    uint32_t remaining_ms = timer_elapsed_ms;
    if (timer_state == TIMER_STATE_RUNNING) {
        uint32_t elapsed_since_start = lv_tick_get() - timer_started_at;
        remaining_ms = elapsed_since_start >= remaining_ms
                           ? 0
                           : remaining_ms - elapsed_since_start;
    }

    uint32_t total_seconds = remaining_ms / 1000U;
    uint32_t hours = total_seconds / 3600U;
    uint32_t minutes = (total_seconds % 3600U) / 60U;
    uint32_t seconds = total_seconds % 60U;

    char hours_text[TIMER_HOURS_DIGITS + 1];
    char minutes_text[TIMER_MINUTES_DIGITS + 1];
    char seconds_text[TIMER_SECONDS_DIGITS + 1];
    snprintf(hours_text, sizeof(hours_text), "%02lu", (unsigned long)hours);
    snprintf(minutes_text, sizeof(minutes_text), "%02lu", (unsigned long)minutes);
    snprintf(seconds_text, sizeof(seconds_text), "%02lu", (unsigned long)seconds);
    lv_label_set_text(timer_hours_label, hours_text);
    lv_label_set_text(timer_minutes_label, minutes_text);
    lv_label_set_text(timer_seconds_label, seconds_text);
}

static void timer_update_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    if (timer_state == TIMER_STATE_RUNNING) {
        uint32_t elapsed_since_start = lv_tick_get() - timer_started_at;
        if (elapsed_since_start >= timer_elapsed_ms) {
            timer_elapsed_ms = 0;
            timer_state = TIMER_STATE_IDLE;
            if (timer_control_label != NULL) {
                lv_label_set_text(timer_control_label, "Start");
            }
        }
    }
    timer_update_display();
}

static bool timer_init(void)
{
    timer_state = TIMER_STATE_IDLE;
    timer_elapsed_ms = 0;
    timer_started_at = 0;
    timer_setted_ms = 0;
    timer_hours_label = NULL;
    timer_minutes_label = NULL;
    timer_seconds_label = NULL;
    timer_control_label = NULL;
    timer_update_timer = NULL;
    return true;
}

static void timer_back_clicked(lv_event_t *event)
{
    (void)event;
    application_manager_close();
}

static void timer_reset_clicked(lv_event_t *event)
{
    (void)event;
    if (timer_state == TIMER_STATE_IDLE) {
        return;
    }
    else if (timer_state == TIMER_STATE_SETTED) {
        timer_elapsed_ms = 0;
        timer_setted_ms = 0;
        timer_state = TIMER_STATE_IDLE;
    } else if (timer_state == TIMER_STATE_RUNNING) {
        timer_elapsed_ms = timer_setted_ms;
        timer_started_at = lv_tick_get();
    } else if (timer_state == TIMER_STATE_PAUSED) {
        timer_setted_ms = 0;
        timer_elapsed_ms = 0;
    }
    timer_update_display();
}

static void timer_continue_pause_clicked(lv_event_t *event)
{
    (void)event;
    if (timer_state == TIMER_STATE_IDLE) {
        if (timer_control_label != NULL) lv_label_set_text(timer_control_label, "Start");
    } else if (timer_state == TIMER_STATE_SETTED) {
        timer_elapsed_ms = timer_setted_ms;
        if (timer_elapsed_ms == 0) return;
        timer_started_at = lv_tick_get();
        timer_state = TIMER_STATE_RUNNING;
        if (timer_control_label != NULL) lv_label_set_text(timer_control_label, "Pause");
    } else if (timer_state == TIMER_STATE_RUNNING) {
        uint32_t elapsed_since_start = lv_tick_get() - timer_started_at;
        timer_elapsed_ms = elapsed_since_start >= timer_elapsed_ms
                               ? 0
                               : timer_elapsed_ms - elapsed_since_start;
        timer_state = timer_elapsed_ms == 0 ? TIMER_STATE_IDLE : TIMER_STATE_PAUSED;
        if (timer_control_label != NULL) {
            lv_label_set_text(timer_control_label,
                              timer_state == TIMER_STATE_IDLE ? "Start" : "Continue");
        }
    } else if (timer_state == TIMER_STATE_PAUSED) {
        if (timer_elapsed_ms == 0) {
            timer_state = TIMER_STATE_IDLE;
            if (timer_control_label != NULL) {
                lv_label_set_text(timer_control_label, "Start");
            }
            timer_update_display();
            return;
        }
        timer_started_at = lv_tick_get();
        timer_state = TIMER_STATE_RUNNING;
        if (timer_control_label != NULL) lv_label_set_text(timer_control_label, "Pause");
    }
    timer_update_display();
}

static void timer_adjust_clicked(lv_event_t *event)
{
    const timer_adjust_data_t *data = lv_event_get_user_data(event);
    if (data == NULL || data->unit >= TIMER_UNIT_COUNT ||
        timer_state == TIMER_STATE_RUNNING) return;

    if (timer_hours_label == NULL || timer_minutes_label == NULL ||
        timer_seconds_label == NULL) return;

    lv_obj_t *labels[TIMER_UNIT_COUNT] = {
        timer_hours_label, timer_minutes_label, timer_seconds_label
    };
    static const uint8_t max_values[TIMER_UNIT_COUNT] = {99, 59, 59};
    lv_obj_t *label = labels[data->unit];
    if (label == NULL) return;

    int value = (int)strtoul(lv_label_get_text(label), NULL, 10) + data->delta;
    if (value < 0) value = max_values[data->unit];
    if (value > max_values[data->unit]) value = 0;

    char text[3];
    snprintf(text, sizeof(text), "%02d", value);
    lv_label_set_text(label, text);

    timer_state = TIMER_STATE_SETTED;
    timer_setted_ms = (
        (uint32_t)strtoul(lv_label_get_text(timer_hours_label), NULL, 10) * 3600U +
        (uint32_t)strtoul(lv_label_get_text(timer_minutes_label), NULL, 10) * 60U +
        (uint32_t)strtoul(lv_label_get_text(timer_seconds_label), NULL, 10)) * 1000U;
    timer_elapsed_ms = timer_setted_ms;
    if (timer_control_label != NULL) lv_label_set_text(timer_control_label, "Start");
    timer_update_display();
}

static void timer_create_adjust_button(lv_obj_t *parent, lv_obj_t *target,
                                       timer_unit_t unit, int8_t delta,
                                       const char *symbol, lv_align_t align,
                                       int y_offset)
{
    static timer_adjust_data_t event_data[TIMER_UNIT_COUNT][2];
    uint8_t direction = delta > 0 ? 0 : 1;
    event_data[unit][direction] = (timer_adjust_data_t){unit, delta};

    lv_obj_t *button = lv_btn_create(parent);
    lv_obj_set_size(button, TIMER_ADJUST_BUTTON_WIDTH, TIMER_ADJUST_BUTTON_HEIGHT);
    lv_obj_set_style_shadow_width(button, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(button, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(button, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(button, COLOR_PRIMARY, LV_PART_MAIN);
    lv_obj_set_style_radius(button, RADIUS_DEFAULT, LV_PART_MAIN);
    lv_obj_add_event_cb(button, timer_adjust_clicked, LV_EVENT_CLICKED,
                        &event_data[unit][direction]);
    lv_obj_align_to(button, target, align, 0, y_offset);

    lv_obj_t *label = lv_label_create(button);
    lv_label_set_text(label, symbol);
    lv_obj_set_style_text_font(label, FONT_SIZE_12, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, COLOR_MAIN_CONTENT, LV_PART_MAIN);
    lv_obj_center(label);
}

static lv_obj_t *timer_create_ui(void)
{
    lv_obj_t *screen = lv_obj_create(NULL);
    if (screen == NULL) return NULL;

    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(screen, COLOR_BACKGROUND, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);
    ui_status_bar_create(screen);

    // back button
    lv_obj_t *back_button = lv_btn_create(screen);
    lv_obj_set_size(back_button, 90, 20);
    lv_obj_align(back_button, LV_ALIGN_BOTTOM_LEFT, 12, -12);
    lv_obj_add_event_cb(back_button, timer_back_clicked, LV_EVENT_CLICKED, NULL);
    lv_obj_t *back_label = lv_label_create(back_button);
    lv_label_set_text(back_label, "Back");
    lv_obj_set_style_text_font(back_label, FONT_SIZE_12, LV_PART_MAIN);
    lv_obj_center(back_label);

    const lv_coord_t digit_width = (lv_coord_t)lv_font_get_glyph_width(
        CONTENT_TIMER_FONT, '0', '0');
    const lv_coord_t colon_width = (lv_coord_t)lv_font_get_glyph_width(
        CONTENT_TIMER_FONT, ':', ':');
    const lv_coord_t hours_width = digit_width * TIMER_HOURS_DIGITS;
    const lv_coord_t minutes_width = digit_width * TIMER_MINUTES_DIGITS;
    const lv_coord_t seconds_width = digit_width * TIMER_SECONDS_DIGITS;
    const lv_coord_t group_width = hours_width + minutes_width + seconds_width +
                                   colon_width * 2 + TIMER_COLON_GAP * 4;

    lv_obj_t *time_row = lv_obj_create(screen);
    lv_obj_remove_style_all(time_row);
    lv_obj_clear_flag(time_row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(time_row, group_width, LV_SIZE_CONTENT);
    lv_obj_align(time_row, LV_ALIGN_TOP_MID, 0, 70);

    timer_hours_label = lv_label_create(time_row);
    lv_label_set_text(timer_hours_label, "00");
    lv_obj_set_width(timer_hours_label, hours_width);
    lv_obj_set_style_text_font(timer_hours_label, CONTENT_TIMER_FONT, LV_PART_MAIN);
    lv_obj_set_style_text_color(timer_hours_label, COLOR_MAIN_CONTENT, LV_PART_MAIN);
    lv_obj_set_style_text_align(timer_hours_label, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
    lv_obj_align(timer_hours_label, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_t *first_colon = lv_label_create(time_row);
    lv_label_set_text(first_colon, ":");
    lv_obj_set_width(first_colon, colon_width);
    lv_obj_set_style_text_font(first_colon, CONTENT_TIMER_FONT, LV_PART_MAIN);
    lv_obj_set_style_text_color(first_colon, COLOR_MAIN_CONTENT, LV_PART_MAIN);
    lv_obj_set_style_text_align(first_colon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    const lv_coord_t first_colon_x = hours_width + TIMER_COLON_GAP;
    lv_obj_align(first_colon, LV_ALIGN_TOP_LEFT, first_colon_x, 0);

    timer_minutes_label = lv_label_create(time_row);
    lv_label_set_text(timer_minutes_label, "00");
    lv_obj_set_width(timer_minutes_label, minutes_width);
    lv_obj_set_style_text_font(timer_minutes_label, CONTENT_TIMER_FONT, LV_PART_MAIN);
    lv_obj_set_style_text_color(timer_minutes_label, COLOR_MAIN_CONTENT, LV_PART_MAIN);
    lv_obj_set_style_text_align(timer_minutes_label, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
    const lv_coord_t minutes_x = first_colon_x + colon_width + TIMER_COLON_GAP;
    lv_obj_align(timer_minutes_label, LV_ALIGN_TOP_LEFT, minutes_x, 0);

    lv_obj_t *second_colon = lv_label_create(time_row);
    lv_label_set_text(second_colon, ":");
    lv_obj_set_width(second_colon, colon_width);
    lv_obj_set_style_text_font(second_colon, CONTENT_TIMER_FONT, LV_PART_MAIN);
    lv_obj_set_style_text_color(second_colon, COLOR_MAIN_CONTENT, LV_PART_MAIN);
    lv_obj_set_style_text_align(second_colon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    const lv_coord_t second_colon_x = minutes_x + minutes_width + TIMER_COLON_GAP;
    lv_obj_align(second_colon, LV_ALIGN_TOP_LEFT, second_colon_x, 0);

    timer_seconds_label = lv_label_create(time_row);
    lv_label_set_text(timer_seconds_label, "00");
    lv_obj_set_width(timer_seconds_label, seconds_width);
    lv_obj_set_style_text_font(timer_seconds_label, CONTENT_TIMER_FONT, LV_PART_MAIN);
    lv_obj_set_style_text_color(timer_seconds_label, COLOR_MAIN_CONTENT, LV_PART_MAIN);
    lv_obj_set_style_text_align(timer_seconds_label, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    const lv_coord_t seconds_x = second_colon_x + colon_width + TIMER_COLON_GAP;
    lv_obj_align(timer_seconds_label, LV_ALIGN_TOP_LEFT, seconds_x, 0);

    lv_obj_t *unit_labels[TIMER_UNIT_COUNT] = {
        timer_hours_label, timer_minutes_label, timer_seconds_label
    };
    for (uint8_t i = 0; i < TIMER_UNIT_COUNT; i++) {
        /* Keep controls on the screen layer: time_row only sizes itself to the
         * digits, so controls extending above/below it can otherwise be clipped. */
        timer_create_adjust_button(screen, unit_labels[i], (timer_unit_t)i,
                                   1, LV_SYMBOL_UP, LV_ALIGN_OUT_TOP_MID, -5);
        timer_create_adjust_button(screen, unit_labels[i], (timer_unit_t)i,
                                   -1, LV_SYMBOL_DOWN, LV_ALIGN_OUT_BOTTOM_MID, 4);
    }

    lv_obj_t *control_button = lv_btn_create(screen);
    lv_obj_set_size(control_button, 120, 40);
    lv_obj_align(control_button, LV_ALIGN_LEFT_MID, 30, 70);
    lv_obj_add_event_cb(control_button, timer_continue_pause_clicked, LV_EVENT_CLICKED, NULL);
    timer_control_label = lv_label_create(control_button);
    lv_label_set_text(timer_control_label, "Start");
    lv_obj_set_style_text_font(timer_control_label, CONTENT_TIMER_CONTROL_FONT, LV_PART_MAIN);
    lv_obj_center(timer_control_label);

    lv_obj_t *reset_button = lv_btn_create(screen);
    lv_obj_set_size(reset_button, 120, 40);
    lv_obj_align(reset_button, LV_ALIGN_RIGHT_MID, -30, 70);
    lv_obj_add_event_cb(reset_button, timer_reset_clicked, LV_EVENT_CLICKED, NULL);
    lv_obj_t *reset_label = lv_label_create(reset_button);
    lv_label_set_text(reset_label, "Reset");
    lv_obj_set_style_text_font(reset_label, CONTENT_TIMER_CONTROL_FONT, LV_PART_MAIN);
    lv_obj_center(reset_label);

    timer_update_timer = lv_timer_create(timer_update_timer_cb,
                                         TIMER_UPDATE_PERIOD_MS, NULL);
    if (timer_update_timer == NULL) return NULL;

    return screen;
}

static void timer_on_enter(void) {
    // for preloaded task when user start app
    // like in weather is update new weather information
}

static void timer_on_pause(void) {
    // will be use if app is allow to run in background
}
static void timer_on_resume(void) {
    // will be use if app is allow to run in background
}

static void timer_on_exit(void) {
    // back to home screen, maybe just it
}

static void timer_deinit(void) {
    if (timer_update_timer != NULL) {
        lv_timer_del(timer_update_timer);
        timer_update_timer = NULL;
    }
}

const app_descriptor_t app_timer = {
    .app_id = "timer",
    .app_name = "Timer",
    .start_tile = {
        .icon = &timer_icon,
        .color_hex = 0xF28A12,
        .size = APP_TILE_SIZE_SMALL,
        .x = 163,
        .y = 216,
        .title = NULL,
        .content = NULL,
    },
    .init = timer_init,
    .create_ui = timer_create_ui,
    .on_enter = timer_on_enter,
    .on_pause = timer_on_pause,
    .on_resume = timer_on_resume,
    .on_exit = timer_on_exit,
    .deinit = timer_deinit,
};


#ifdef LVGL_LIVE_PREVIEW
void lvgl_live_preview_init(void)
{
    timer_init();
    lv_obj_t *screen = timer_create_ui();
    if (screen != NULL) lv_scr_load(screen);
}
#endif
