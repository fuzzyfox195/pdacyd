#ifndef STATUS_BAR_H
#define STATUS_BAR_H

#include "lvgl.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void ui_status_bar_create(lv_obj_t *parent);
void ui_status_bar_set_battery_adc(int adc_raw);
void ui_status_bar_set_time(uint8_t hour, uint8_t minute, bool valid);

#ifdef __cplusplus
}
#endif

#endif
