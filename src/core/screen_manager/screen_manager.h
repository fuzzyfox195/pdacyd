#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H
#include "lvgl.h"
#ifdef __cplusplus
extern "C" {
#endif
void screen_manager_show(lv_obj_t *screen);
void screen_manager_show_home(void);
void screen_manager_show_boot_lock(void);
void screen_manager_show_sleep_lock(void);
bool screen_manager_unlock(void);
bool screen_manager_is_locked(void);
bool screen_manager_push_modal(lv_obj_t *screen);
bool screen_manager_pop_modal(void);
bool screen_manager_has_modal(void);
#ifdef __cplusplus
}
#endif
#endif
