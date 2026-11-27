#include "app_calendar.h"
#include "../../core/application_manager/application_manager.h"
#include "../../ui/assets/include_theme.h"
#include "../../ui/status_bar/status_bar.h"

extern const lv_img_dsc_t calendar_icon;

static bool calendar_init(void)
{
    return true;
}

static lv_obj_t *calendar_create_ui(void)
{
    lv_obj_t *screen = lv_obj_create(NULL);
    if (screen == NULL) return NULL;

    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(screen, COLOR_BACKGROUND, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);
    ui_status_bar_create(screen);

    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "Calendar");
    lv_obj_set_style_text_font(title, HEADING_FONT, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, COLOR_MAIN_CONTENT, LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 12, 44);

    lv_obj_t *message = lv_label_create(screen);
    lv_label_set_text(message, "The Calendar app is coming soon.");
    lv_obj_set_style_text_font(message, CONTENT_FONT, LV_PART_MAIN);
    lv_obj_set_style_text_color(message, COLOR_HINT_GRAY, LV_PART_MAIN);
    lv_obj_align(message, LV_ALIGN_CENTER, 0, 0);

    return screen;
}

static void calendar_on_enter(void) {}
static void calendar_on_pause(void) {}
static void calendar_on_resume(void) {}
static void calendar_on_exit(void) {}
static void calendar_deinit(void) {}

const app_descriptor_t app_calendar = {
    .app_id = "calendar",
    .app_name = "Calendar",
    .start_tile = {
        .icon = &calendar_icon,
        .color_hex = 0xF28A12,
        .size = APP_TILE_SIZE_LARGE,
        .x = 8,
        .y = 216,
        .title = "-- ---",
        .content = "Waiting for time",
    },
    .init = calendar_init,
    .create_ui = calendar_create_ui,
    .on_enter = calendar_on_enter,
    .on_pause = calendar_on_pause,
    .on_resume = calendar_on_resume,
    .on_exit = calendar_on_exit,
    .deinit = calendar_deinit,
};
