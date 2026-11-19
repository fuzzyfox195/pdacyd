#ifndef START_SCREEN_H
#define START_SCREEN_H

#include "lvgl.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void ui_start_screen_screen_init(void);
bool ui_start_screen_update_tile(const char *app_id, const char *title,
                                 const char *content);

#ifdef __cplusplus
}
#endif

#endif // START_SCREEN_H
