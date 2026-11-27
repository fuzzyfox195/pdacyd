#include "app_setting.h"

#include "connection/setting_connection.h"
#include "theme/setting_theme.h"
#include "ringtone/setting_ringtone.h"
#include "battery/setting_battery.h"
#include "time/setting_time.h"
#include "information/setting_information.h"
#include "i2c_scan/setting_i2c_scan.h"
#include "sleep/setting_sleep.h"
#include "button_debug/setting_button_debug.h"
#include "../../ui/screens/sleep_options/sleep_options.h"
#include "setting_option_ui.h"
#include "../../core/application_manager/application_manager.h"
#include "../../ui/assets/include_theme.h"
#include "../../ui/status_bar/status_bar.h"

#include <stdint.h>

extern const lv_img_dsc_t setting_icon;
extern const lv_img_dsc_t icon_battery;
extern const lv_img_dsc_t icon_connection;
extern const lv_img_dsc_t icon_infor;
extern const lv_img_dsc_t icon_ringtone;
extern const lv_img_dsc_t icon_theme;
extern const lv_img_dsc_t icon_time;
extern const lv_img_dsc_t icon_i2c_scan;
extern const lv_img_dsc_t icon_sleep;
extern const lv_img_dsc_t icon_button;

#define SETTING_TILE_MARGIN 8
#define SETTING_TILE_GAP 8
#define SETTING_TILE_HEIGHT 46
#define SETTING_ICON_SIZE 24
#define SETTING_ICON_MARGIN 5
#define SETTING_TITLE_GAP 4
#define SETTING_VIEWPORT_TOP 30

typedef struct {
    const char *title;
    const lv_img_dsc_t *icon;
    uint32_t color_hex;
    setting_option_create_fn create_ui;
    void (*close_ui)(void);
    bool full_screen;
} setting_option_t;

static const setting_option_t setting_options[] = {
    {"Connection", &icon_connection, 0x2583E8, setting_connection_create_ui, NULL, false},
    {"Theme", &icon_theme, 0x8757D6, setting_theme_create_ui, NULL, false},
    {"Ringtone", &icon_ringtone, 0xD4657A, setting_ringtone_create_ui, NULL, false},
    {"Battery", &icon_battery, 0x40A878, setting_battery_create_ui, NULL, false},
    {"Time", &icon_time, 0xD68A35, setting_time_create_ui, NULL, false},
    {"I2C Scan", &icon_i2c_scan, 0x6E82A5, setting_i2c_scan_create_ui,
     setting_i2c_scan_close_ui, false},
    {"Sleep mode", &icon_sleep, 0x5267C9, setting_sleep_create_ui, NULL, true},
    {"Button Debug", &icon_button, 0x2D8A9E,
     setting_button_debug_create_ui, setting_button_debug_close_ui, false},
    {"Information", &icon_infor, 0x4C9A9A, setting_information_create_ui, NULL, false},
};

#define SETTING_OPTION_COUNT (sizeof(setting_options) / sizeof(setting_options[0]))

static lv_obj_t *setting_screen;
static lv_obj_t *setting_page;
static bool option_open;
static size_t selected_option;

static void setting_option_clicked(lv_event_t *event)
{
    const size_t option_index = (size_t)(uintptr_t)lv_event_get_user_data(event);
    if (option_index >= SETTING_OPTION_COUNT || setting_page == NULL) return;

    if (setting_options[option_index].full_screen) {
        setting_options[option_index].create_ui(setting_page);
        return;
    }

    lv_obj_clean(setting_page);
    option_open = true;
    selected_option = option_index;
    setting_options[option_index].create_ui(setting_page);
}

void setting_show_menu(void)
{
    if (setting_page == NULL) return;

    if (option_open && setting_options[selected_option].close_ui != NULL) {
        setting_options[selected_option].close_ui();
    }

    option_open = false;
    lv_obj_clean(setting_page);

    const lv_coord_t screen_width = lv_disp_get_hor_res(lv_disp_get_default());
    const lv_coord_t tile_width = (screen_width - 2 * SETTING_TILE_MARGIN -
                                   SETTING_TILE_GAP) / 2;

    lv_obj_t *scroll_area = lv_obj_create(setting_page);
    lv_obj_remove_style_all(scroll_area);
    lv_obj_set_size(scroll_area, screen_width, lv_pct(100));
    lv_obj_set_style_bg_opa(scroll_area, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_scroll_dir(scroll_area, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(scroll_area, LV_SCROLLBAR_MODE_OFF);

    const size_t rows = (SETTING_OPTION_COUNT + 1) / 2;
    const lv_coord_t content_height = SETTING_TILE_MARGIN +
        (lv_coord_t)rows * SETTING_TILE_HEIGHT +
        (lv_coord_t)(rows - 1) * SETTING_TILE_GAP + SETTING_TILE_MARGIN;
    lv_obj_t *content = lv_obj_create(scroll_area);
    lv_obj_remove_style_all(content);
    lv_obj_set_size(content, screen_width, content_height);
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);

    for (size_t i = 0; i < SETTING_OPTION_COUNT; ++i) {
        const size_t column = i % 2;
        const size_t row = i / 2;
        const lv_coord_t x = SETTING_TILE_MARGIN +
                             (lv_coord_t)column * (tile_width + SETTING_TILE_GAP);
        const lv_coord_t y = SETTING_TILE_MARGIN +
                             (lv_coord_t)row * (SETTING_TILE_HEIGHT + SETTING_TILE_GAP);

        lv_obj_t *button = lv_obj_create(content);
        lv_obj_remove_style_all(button);
        lv_obj_set_size(button, tile_width, SETTING_TILE_HEIGHT);
        lv_obj_set_pos(button, x, y);
        lv_obj_set_style_bg_color(button, COLOR_BACKGROUND, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(button, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_border_width(button, 1, LV_PART_MAIN);
        lv_obj_set_style_border_color(button,
                                      lv_color_hex(setting_options[i].color_hex), LV_PART_MAIN);
        lv_obj_set_style_radius(button, RADIUS_APP_TILE, LV_PART_MAIN);
        lv_obj_clear_flag(button, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(button, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(button, setting_option_clicked, LV_EVENT_CLICKED,
                            (void *)(uintptr_t)i);

        lv_obj_t *title = lv_label_create(button);
        lv_label_set_text(title, setting_options[i].title);
        lv_obj_set_width(title, tile_width - 10 - SETTING_ICON_SIZE -
                               SETTING_TITLE_GAP);
        lv_obj_set_style_text_color(title, lv_color_hex(setting_options[i].color_hex),
                                    LV_PART_MAIN);
        lv_obj_set_style_text_font(title, &lv_font_montserrat_18, LV_PART_MAIN);
        lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
        lv_obj_align(title, LV_ALIGN_LEFT_MID, 5, 0);
        lv_obj_clear_flag(title, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

        lv_obj_t *icon = lv_img_create(button);
        lv_img_set_src(icon, setting_options[i].icon);
        lv_obj_set_style_img_recolor(icon, lv_color_hex(setting_options[i].color_hex),
                                     LV_PART_MAIN);
        lv_obj_set_style_img_recolor_opa(icon, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_align(icon, LV_ALIGN_RIGHT_MID, -SETTING_ICON_MARGIN, 0);
        lv_obj_clear_flag(icon, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    }
}

static bool setting_init(void)
{
    setting_screen = NULL;
    setting_page = NULL;
    option_open = false;
    selected_option = 0;
    return true;
}

static lv_obj_t *setting_create_ui(void)
{
    setting_screen = lv_obj_create(NULL);
    if (setting_screen == NULL) return NULL;

    lv_obj_clear_flag(setting_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(setting_screen, COLOR_BACKGROUND, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(setting_screen, LV_OPA_COVER, LV_PART_MAIN);
    ui_status_bar_create(setting_screen);

    const lv_coord_t screen_width = lv_disp_get_hor_res(lv_disp_get_default());
    const lv_coord_t screen_height = lv_disp_get_ver_res(lv_disp_get_default());

    setting_page = lv_obj_create(setting_screen);
    lv_obj_remove_style_all(setting_page);
    lv_obj_set_pos(setting_page, 0, SETTING_VIEWPORT_TOP);
    lv_obj_set_size(setting_page, screen_width,
                    screen_height - SETTING_VIEWPORT_TOP);
    lv_obj_clear_flag(setting_page, LV_OBJ_FLAG_SCROLLABLE);

    setting_show_menu();
    return setting_screen;
}

static bool setting_on_back(void)
{
    if (!option_open) return false;
    setting_show_menu();
    return true;
}

static void setting_on_enter(void) {}
static void setting_on_pause(void) {}
static void setting_on_resume(void) {}
static void setting_on_exit(void)
{
    if (option_open && setting_options[selected_option].close_ui != NULL) {
        setting_options[selected_option].close_ui();
    }
    option_open = false;
}

static void setting_deinit(void)
{
    setting_screen = NULL;
    setting_page = NULL;
    option_open = false;
}

const app_descriptor_t app_setting = {
    .app_id = "setting",
    .app_name = "Setting",
    .start_tile = {
        .icon = &setting_icon,
        .color_hex = 0x444444,
        .size = APP_TILE_SIZE_SMALL,
        .x = 163,
        .y = 108,
        .title = NULL,
        .content = NULL,
    },
    .init = setting_init,
    .create_ui = setting_create_ui,
    .on_enter = setting_on_enter,
    .on_pause = setting_on_pause,
    .on_resume = setting_on_resume,
    .on_exit = setting_on_exit,
    .deinit = setting_deinit,
    .on_back = setting_on_back,
};

#ifdef LVGL_LIVE_PREVIEW
void lvgl_live_preview_init(void)
{
    setting_init();
    lv_obj_t *screen = setting_create_ui();
    if (screen != NULL) {
        lv_scr_load(screen);
        setting_on_enter();
    }
}
#endif
