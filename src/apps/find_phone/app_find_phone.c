#include "app_find_phone.h"
#include "../../core/application_manager/application_manager.h"
#include "../../ui/assets/include_theme.h"
#include "../../ui/status_bar/status_bar.h"

static bool findphone_init(void)
{
    return true;
}

static void findphone_back_clicked(lv_event_t *event)
{
    (void)event;
    application_manager_close();
}

static lv_obj_t *findphone_create_ui(void)
{
    lv_obj_t *screen = lv_obj_create(NULL);
    if (screen == NULL) return NULL;

    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(screen, COLOR_BACKGROUND, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);
    ui_status_bar_create(screen);

    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "Find phone");
    lv_obj_set_style_text_font(title, HEADING_FONT, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, COLOR_MAIN_CONTENT, LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 12, 44);

    lv_obj_t *message = lv_label_create(screen);
    lv_label_set_text(message, "The Find phone app is coming soon.");
    lv_obj_set_style_text_font(message, CONTENT_FONT, LV_PART_MAIN);
    lv_obj_set_style_text_color(message, COLOR_HINT_GRAY, LV_PART_MAIN);
    lv_obj_align(message, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *back_button = lv_btn_create(screen);
    lv_obj_set_size(back_button, 90, 38);
    lv_obj_align(back_button, LV_ALIGN_BOTTOM_LEFT, 12, -12);
    lv_obj_add_event_cb(back_button, findphone_back_clicked, LV_EVENT_CLICKED, NULL);

    lv_obj_t *back_label = lv_label_create(back_button);
    lv_label_set_text(back_label, "Back");
    lv_obj_center(back_label);

    return screen;
}

static void findphone_on_enter(void) {}
static void findphone_on_pause(void) {}
static void findphone_on_resume(void) {}
static void findphone_on_exit(void) {}
static void findphone_deinit(void) {}

const app_descriptor_t app_find_phone = {
    .app_id = "find_phone",
    .app_name = "Find phone",
    .init = findphone_init,
    .create_ui = findphone_create_ui,
    .on_enter = findphone_on_enter,
    .on_pause = findphone_on_pause,
    .on_resume = findphone_on_resume,
    .on_exit = findphone_on_exit,
    .deinit = findphone_deinit,
};
