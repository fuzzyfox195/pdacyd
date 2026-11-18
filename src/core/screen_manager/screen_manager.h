#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H
#include "lvgl.h"
#ifdef __cplusplus
extern "C" {
#endif
void screen_manager_show(lv_obj_t *screen);
void screen_manager_show_home(void);
#ifdef __cplusplus
}
#endif
#endif
