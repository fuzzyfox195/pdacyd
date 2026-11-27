#ifndef SETTING_I2C_SCAN_H
#define SETTING_I2C_SCAN_H

#include "../setting_option_ui.h"

#ifdef __cplusplus
extern "C" {
#endif

void setting_i2c_scan_create_ui(lv_obj_t *parent);
void setting_i2c_scan_close_ui(void);

#ifdef __cplusplus
}
#endif

#endif /* SETTING_I2C_SCAN_H */
