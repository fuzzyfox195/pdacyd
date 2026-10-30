#ifndef MAIN_UI_H
#define MAIN_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

extern lv_obj_t * ui_lock_screen;
extern lv_obj_t * ui_start_screen;

void ui_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif // MAIN_UI_H
