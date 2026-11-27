#include "screen_manager.h"
#include "../../ui/main_ui.h"
#include "../../ui/screens/lock_screen/lock_screen.h"

static lv_obj_t *modal_return_screen;
static lv_obj_t *modal_screen;
static lv_obj_t *lock_resume_screen;
static bool lock_resume_pending;
static bool lock_active;

void screen_manager_show(lv_obj_t *screen)
{
    if (screen != NULL) {
        lv_disp_load_scr(screen);
    }
}

void screen_manager_show_home(void)
{
    screen_manager_show(ui_start_screen);
}

void screen_manager_show_boot_lock(void)
{
    lock_resume_screen = NULL;
    lock_resume_pending = false;
    lock_active = true;
    ui_lock_screen_set_resume_mode(false);
    screen_manager_show(ui_lock_screen);
}

void screen_manager_show_sleep_lock(void)
{
    if (!lock_active) {
        lock_resume_screen = modal_screen != NULL ? modal_return_screen : lv_scr_act();
        lock_resume_pending = lock_resume_screen != NULL &&
                              lock_resume_screen != ui_lock_screen;
    }

    lv_obj_t *old_modal = modal_screen;
    modal_screen = NULL;
    modal_return_screen = NULL;
    lock_active = true;
    ui_lock_screen_set_resume_mode(lock_resume_pending);
    screen_manager_show(ui_lock_screen);
    if (old_modal != NULL) lv_obj_del(old_modal);
}

bool screen_manager_unlock(void)
{
    if (!lock_active) return false;
    const bool resume_previous = lock_resume_pending && lock_resume_screen != NULL;
    lv_obj_t *target = resume_previous ? lock_resume_screen : ui_start_screen;
    lock_active = false;
    lock_resume_pending = false;
    lock_resume_screen = NULL;
    screen_manager_show(target);
    return resume_previous;
}

bool screen_manager_is_locked(void)
{
    return lock_active;
}

bool screen_manager_push_modal(lv_obj_t *screen)
{
    if (screen == NULL || modal_screen != NULL) return false;
    modal_return_screen = lv_scr_act();
    modal_screen = screen;
    screen_manager_show(screen);
    return true;
}

bool screen_manager_pop_modal(void)
{
    if (modal_screen == NULL || modal_return_screen == NULL) return false;
    lv_obj_t *screen = modal_screen;
    screen_manager_show(modal_return_screen);
    modal_screen = NULL;
    modal_return_screen = NULL;
    lv_obj_del(screen);
    return true;
}

bool screen_manager_has_modal(void)
{
    return modal_screen != NULL;
}
