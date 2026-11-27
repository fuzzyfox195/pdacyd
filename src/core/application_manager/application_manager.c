#include "application_manager.h"
#include "app_interface.h"
#include "../../apps/app_catalog.h"
#include "../screen_manager/screen_manager.h"

static const app_descriptor_t *current_app;
static lv_obj_t *current_screen;
static bool initialized;
static bool paused_for_sleep;

void application_manager_init(void)
{
    current_app = NULL;
    current_screen = NULL;
    initialized = true;
    paused_for_sleep = false;
}

bool application_manager_launch(const char *app_id)
{
    if (!initialized || app_id == NULL || current_app != NULL) return false;
    const app_descriptor_t *app = app_catalog_find(app_id);
    if (app == NULL || app->init == NULL || app->create_ui == NULL ||
        app->deinit == NULL) return false;

    if (!app->init()) {
        app->deinit();
        return false;
    }
    lv_obj_t *screen = app->create_ui();
    if (screen == NULL) {
        app->deinit();
        return false;
    }

    current_app = app;
    current_screen = screen;
    screen_manager_show(screen);
    if (app->on_enter != NULL) app->on_enter();
    return true;
}

bool application_manager_back(void)
{
    if (current_app == NULL) return false;
    if (current_app->on_back != NULL && current_app->on_back()) return true;
    application_manager_close();
    return true;
}

void application_manager_home(void)
{
    if (current_app != NULL) {
        application_manager_close();
    } else {
        screen_manager_show_home();
    }
}

void application_manager_option(void)
{
    if (current_app != NULL && current_app->on_option != NULL) {
        current_app->on_option();
    }
}

void application_manager_favorite(void)
{
    if (current_app != NULL && current_app->on_favorite != NULL) {
        current_app->on_favorite();
    }
}

void application_manager_prepare_sleep(void)
{
    if (current_app == NULL || paused_for_sleep) return;
    paused_for_sleep = true;
    if (current_app->on_pause != NULL) current_app->on_pause();
}

void application_manager_resume_after_sleep(void)
{
    if (current_app == NULL || !paused_for_sleep) return;
    paused_for_sleep = false;
    if (current_app->on_resume != NULL) current_app->on_resume();
}

void application_manager_close(void)
{
    if (current_app == NULL) return;
    const app_descriptor_t *app = current_app;
    lv_obj_t *screen = current_screen;

    if (app->on_exit != NULL) app->on_exit();
    paused_for_sleep = false;
    screen_manager_show_home();
    current_app = NULL;
    current_screen = NULL;
    if (screen != NULL) lv_obj_del(screen);
    app->deinit();
}

const char *application_manager_current_id(void)
{
    return current_app != NULL ? current_app->app_id : NULL;
}
