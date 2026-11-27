#ifndef APP_INTERFACE_H
#define APP_INTERFACE_H

#include "lvgl.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    APP_TILE_SIZE_SMALL,
    APP_TILE_SIZE_LARGE,
} app_tile_size_t;

/* Immutable Start Screen metadata. Strings and icons remain in flash. */
typedef struct {
    const lv_img_dsc_t *icon;
    uint32_t color_hex;
    app_tile_size_t size;
    lv_coord_t x; /* Position relative to the Start Screen scroll content. */
    lv_coord_t y;
    const char *title;
    const char *content;
} app_tile_info_t;

typedef struct app_descriptor {
    const char *app_id;
    const char *app_name;
    app_tile_info_t start_tile;
    bool (*init)(void);
    lv_obj_t *(*create_ui)(void);
    void (*on_enter)(void);
    void (*on_pause)(void);
    void (*on_resume)(void);
    void (*on_exit)(void);
    void (*deinit)(void);
    bool (*on_back)(void); /* Return true when the app handled Back internally. */
    void (*on_option)(void);
    void (*on_favorite)(void);
} app_descriptor_t;

#endif
