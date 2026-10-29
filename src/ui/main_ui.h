#ifndef MAIN_UI_H
#define MAIN_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

// ---- Lock screen ----
extern lv_obj_t * ui_lock_screen;

void ui_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif // MAIN_UI_H
