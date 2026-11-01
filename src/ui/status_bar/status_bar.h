#ifndef STATUS_BAR_H
#define STATUS_BAR_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

void ui_status_bar_create(lv_obj_t *parent);
void ui_status_bar_set_battery_adc(int adc_raw);

#ifdef __cplusplus
}
#endif

#endif
