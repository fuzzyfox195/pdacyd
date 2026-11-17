#ifndef START_SCREEN_H
#define START_SCREEN_H

#include "lvgl.h"
#include <stdbool.h>

/* These are the reference card dimensions. A tile config can override either
 * dimension when a custom-size card is needed. */
#define START_SCREEN_TILE_LARGE_WIDTH  147
#define START_SCREEN_TILE_LARGE_HEIGHT 100
#define START_SCREEN_TILE_SMALL_WIDTH  147
#define START_SCREEN_TILE_SMALL_HEIGHT 46

/* The style chooses the label arrangement; position and dimensions are
 * independent so each tile can be placed or resized in the scroll area. */
typedef enum {
    START_SCREEN_TILE_STYLE_LARGE,
    START_SCREEN_TILE_STYLE_SMALL,
} start_screen_tile_style_t;

/* Coordinates are relative to the scroll area's top-left corner. Colors use
 * 0xRRGGBB so tile configurations can be declared as static data. */
typedef struct {
    const char *id;            /* Stable key used by the runtime update API. */
    const char *app_name;
    const char *title;       /* Optional heading on large tiles. */
    const char *content;     /* Optional description below the heading. */
    uint32_t app_color_hex;
    lv_coord_t x;
    lv_coord_t y;
    lv_coord_t width;
    lv_coord_t height;
    start_screen_tile_style_t style;
} start_screen_tile_config_t;

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
