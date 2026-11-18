#include "start_screen.h"
#include "start_screen_apps.h"
#include "../../main_ui.h"
#include "../../status_bar/status_bar.h"
#include "../../assets/include_theme.h"
#include "../../../core/application_manager/application_manager.h"
#include <string.h>

/* Coordinates are relative to the scroll area's top-left corner. The 8 px
 * gaps and margins match the reference layout; rows can be freely rearranged. */
#define START_SCREEN_RUNTIME_TILE_LIMIT 32
#define START_SCREEN_APP_ICON_SIZE 24
#define START_SCREEN_APP_ICON_MARGIN 5
#define START_SCREEN_APP_NAME_GAP 4
typedef struct {
    const char *id;
    lv_obj_t *title;
    lv_obj_t *content;
    bool is_large;
} tile_runtime_t;
static tile_runtime_t runtime_tiles[START_SCREEN_RUNTIME_TILE_LIMIT];
static uint16_t runtime_tile_count;

static lv_obj_t *create_app_icon(lv_obj_t *tile,
                                 const start_screen_tile_config_t *config,
                                 lv_align_t alignment,
                                 lv_coord_t x_offset,
                                 lv_coord_t y_offset)
{
    if (config->icon == NULL) return NULL;

    lv_obj_t *icon = lv_img_create(tile);
    lv_img_set_src(icon, config->icon);
    lv_obj_set_style_img_recolor(icon,
                                 lv_color_hex(config->app_color_hex),
                                 LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(icon, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_align(icon, alignment, x_offset, y_offset);
    lv_obj_clear_flag(icon, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    return icon;
}

static void create_large_tile_content(lv_obj_t *tile,
                                      const start_screen_tile_config_t *config)
{
    /* Always create both labels so runtime updates can fill initially empty data. */
    lv_obj_t *title = lv_label_create(tile);
    lv_label_set_text(title, config->title != NULL ? config->title : "");
    lv_obj_set_width(title, config->width - 10);
    lv_obj_set_style_text_color(title, COLOR_MAIN_CONTENT, LV_PART_MAIN);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 5, 5);
    lv_obj_clear_flag(title, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *content = lv_label_create(tile);
    lv_label_set_text(content, config->content != NULL ? config->content : "");
    lv_obj_set_width(content, config->width - 10);
    lv_obj_set_style_text_color(content, COLOR_MAIN_CONTENT, LV_PART_MAIN);
    lv_obj_set_style_text_font(content, CONTENT_FONT, LV_PART_MAIN);
    lv_obj_set_style_text_align(content, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_align_to(content, title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);

    /* A missing heading/description is allowed for simple large app tiles. */
    lv_obj_t *app_name = lv_label_create(tile);
    lv_label_set_text(app_name, config->app_name);
    lv_obj_set_width(app_name, config->width - 10 - START_SCREEN_APP_ICON_SIZE -
                                START_SCREEN_APP_NAME_GAP);
    lv_obj_set_style_text_color(app_name, lv_color_hex(config->app_color_hex), LV_PART_MAIN);
    lv_obj_set_style_text_font(app_name, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_set_style_text_align(app_name, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_align(app_name, LV_ALIGN_BOTTOM_LEFT, 5, -5);
    lv_obj_clear_flag(app_name, LV_OBJ_FLAG_SCROLLABLE);
    create_app_icon(tile, config, LV_ALIGN_BOTTOM_RIGHT,
                    -START_SCREEN_APP_ICON_MARGIN,
                    -START_SCREEN_APP_ICON_MARGIN);
    runtime_tiles[runtime_tile_count++] = (tile_runtime_t){config->id, title, content, true};
}

static void create_small_tile_content(lv_obj_t *tile,
                                      const start_screen_tile_config_t *config)
{
    /* Keep the name vertically centered and inset beside the right-aligned icon. */
    lv_obj_t *app_name = lv_label_create(tile);
    lv_label_set_text(app_name, config->app_name);
    lv_obj_set_width(app_name, config->width - 10 - START_SCREEN_APP_ICON_SIZE -
                                START_SCREEN_APP_NAME_GAP);
    lv_obj_set_style_text_color(app_name, lv_color_hex(config->app_color_hex), LV_PART_MAIN);
    lv_obj_set_style_text_font(app_name, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_set_style_text_align(app_name, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_align(app_name, LV_ALIGN_LEFT_MID, 5, 0);
    lv_obj_clear_flag(app_name, LV_OBJ_FLAG_SCROLLABLE);
    create_app_icon(tile, config, LV_ALIGN_RIGHT_MID,
                    -START_SCREEN_APP_ICON_MARGIN, 0);
    runtime_tiles[runtime_tile_count++] = (tile_runtime_t){config->id, NULL, NULL, false};
}

static void app_tile_clicked(lv_event_t *event)
{
    const char *app_id = (const char *)lv_event_get_user_data(event);
    if (app_id != NULL) {
        (void)application_manager_launch(app_id);
    }
}

static void create_app_tile(lv_obj_t *parent,
                            const start_screen_tile_config_t *config)
{
    /* Each tile is a fixed-size, non-scrollable card inside the scrolling list. */
    lv_obj_t *tile = lv_obj_create(parent);
    lv_obj_remove_style_all(tile);
    lv_obj_set_size(tile, config->width, config->height);
    lv_obj_set_pos(tile, config->x, config->y);
    lv_obj_set_style_bg_color(tile, COLOR_BACKGROUND, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(tile, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(tile, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(tile, lv_color_hex(config->app_color_hex), LV_PART_MAIN);
    lv_obj_set_style_radius(tile, RADIUS_APP_TILE, LV_PART_MAIN);
    lv_obj_clear_flag(tile, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(tile, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(tile, app_tile_clicked, LV_EVENT_CLICKED,
                        (void *)config->id);

    if (config->style == START_SCREEN_TILE_STYLE_SMALL) {
        create_small_tile_content(tile, config);
    } else {
        create_large_tile_content(tile, config);
    }
}

void ui_start_screen_screen_init(void)
{
    if (ui_start_screen != NULL) {
        lv_obj_del(ui_start_screen);
        ui_start_screen = NULL;
    }
    ui_start_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_start_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_start_screen, COLOR_BACKGROUND, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ui_start_screen, LV_OPA_COVER, LV_PART_MAIN);
    ui_status_bar_create(ui_start_screen);

    /* This viewport starts 30 px below the screen top and scrolls vertically
     * only. Its child content is sized to the lowest configured tile. */
    const lv_coord_t viewport_top = 30;
    const lv_coord_t display_height = lv_disp_get_ver_res(lv_disp_get_default());
    lv_obj_t *scroll_area = lv_obj_create(ui_start_screen);
    lv_obj_remove_style_all(scroll_area);
    lv_obj_set_pos(scroll_area, 0, viewport_top);
    lv_obj_set_size(scroll_area, lv_disp_get_hor_res(lv_disp_get_default()),
                    display_height - viewport_top);
    lv_obj_set_style_bg_opa(scroll_area, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_scroll_dir(scroll_area, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(scroll_area, LV_SCROLLBAR_MODE_OFF);

    /* Keep the content panel non-scrollable: gestures belong to scroll_area. */
    lv_coord_t content_height = 0;
    runtime_tile_count = 0;
    memset(runtime_tiles, 0, sizeof(runtime_tiles));
    for (uint16_t i = 0; i < start_screen_app_count; ++i) {
        const lv_coord_t tile_bottom = start_screen_apps[i].y + start_screen_apps[i].height;
        if (tile_bottom > content_height) content_height = tile_bottom;
    }
    content_height += 8; /* Leave the same 8 px margin at the bottom. */

    lv_obj_t *content = lv_obj_create(scroll_area);
    lv_obj_remove_style_all(content);
    lv_obj_set_size(content, lv_disp_get_hor_res(lv_disp_get_default()), content_height);
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);

    for (uint16_t i = 0; i < start_screen_app_count &&
         i < START_SCREEN_RUNTIME_TILE_LIMIT; ++i) {
        create_app_tile(content, &start_screen_apps[i]);
    }
}

bool ui_start_screen_update_tile(const char *app_id, const char *title,
                                 const char *content)
{
    if (app_id == NULL) return false;
    for (uint16_t i = 0; i < runtime_tile_count; ++i) {
        tile_runtime_t *tile = &runtime_tiles[i];
        if (strcmp(tile->id, app_id) != 0) continue;
        if (!tile->is_large) return false;
        lv_label_set_text(tile->title, title != NULL ? title : "");
        lv_label_set_text(tile->content, content != NULL ? content : "");
        lv_obj_align_to(tile->content, tile->title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
        return true;
    }
    return false;
}
