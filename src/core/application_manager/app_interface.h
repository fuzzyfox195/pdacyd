#ifndef APP_INTERFACE_H
#define APP_INTERFACE_H

#include "lvgl.h"
#include <stdbool.h>

typedef struct app_descriptor {
    const char *app_id;
    const char *app_name;
    bool (*init)(void);
    lv_obj_t *(*create_ui)(void);
    void (*on_enter)(void);
    void (*on_pause)(void);
    void (*on_resume)(void);
    void (*on_exit)(void);
    void (*deinit)(void);
} app_descriptor_t;

#endif
