#include "app_stopwatch.h"
#include "../../core/application_manager/application_manager.h"
#include "../../ui/assets/include_theme.h"
#include "../../ui/status_bar/status_bar.h"

static bool stopwatch_init(void)
{
    return true;
}

static void stopwatch_back_clicked(lv_event_t *event)
{
    (void)event;
    application_manager_close();
}

static lv_obj_t *stopwatch_create_ui(void)
{
    lv_obj_t *screen = lv_obj_create(NULL);
    if (screen == NULL) return NULL;

    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(screen, COLOR_BACKGROUND, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);
    ui_status_bar_create(screen);


    return screen;
}

static void stopwatch_on_enter(void) {
    // for preloaded task when user start app
    // like in weather is update new weather information
}

static void stopwatch_on_pause(void) {
    // will be use if app is allow to run in background
}

static void stopwatch_on_resume(void) {
    // will be use if app is allow to run in background
}

static void stopwatch_on_exit(void) {
    // back to home screen, maybe just it
}

static void stopwatch_deinit(void) {
    // Use to remove resource used in void init
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
// Gives the live preview tool a way to initialize this UI.
void lvgl_live_preview_init(void)
{
    stopwatch_create_ui();
}
#endif
