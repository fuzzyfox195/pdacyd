#ifndef SETTING_OPTION_UI_H
#define SETTING_OPTION_UI_H

#include "lvgl.h"

typedef void (*setting_option_create_fn)(lv_obj_t *parent);

void setting_option_ui_create(lv_obj_t *parent, const char *title,
                              const char *description);

#endif /* SETTING_OPTION_UI_H */
