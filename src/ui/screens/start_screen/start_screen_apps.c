#include "start_screen_apps.h"

extern const lv_img_dsc_t weather_icon;
extern const lv_img_dsc_t voicenote_icon;
extern const lv_img_dsc_t music_icon;
extern const lv_img_dsc_t setting_icon;
extern const lv_img_dsc_t findphone_icon;
extern const lv_img_dsc_t calendar_icon;
extern const lv_img_dsc_t timer_icon;
extern const lv_img_dsc_t stopwatch_icon;

 /* For calculating the position of each tile 
    let the first tile be at (8, 0) 
    Large tile have size of 147x100
    so the neext tile by vertial will be at (8+147+8=163, 0)
    and the next tile by horizontal will be at (8, 0+100+8=116)
    note that we have large tiles and small tiles, so be careful with the positioning */

const start_screen_tile_config_t start_screen_apps[] = {

    {"weather", "Weather", "Ha Noi", "32 °C, feel like 34 °C",
        &weather_icon,
        0x2583E8, 8, 0, START_SCREEN_TILE_LARGE_WIDTH, START_SCREEN_TILE_LARGE_HEIGHT,
        START_SCREEN_TILE_STYLE_LARGE},

    {"voice_note", "Voice note", "12 notes", "Use your voice to take note",
        &voicenote_icon,
        0x159A79, 163, 0, START_SCREEN_TILE_LARGE_WIDTH, START_SCREEN_TILE_LARGE_HEIGHT,
        START_SCREEN_TILE_STYLE_LARGE},

    {"music", "Komposer", "Local", "Em dau\nPhung Khan Linh...",
        &music_icon,
        0x12B85A, 8, 108, START_SCREEN_TILE_LARGE_WIDTH, START_SCREEN_TILE_LARGE_HEIGHT,
        START_SCREEN_TILE_STYLE_LARGE},

    {"setting", "Setting", NULL, NULL, &setting_icon,
        0x444444, 163, 108, START_SCREEN_TILE_SMALL_WIDTH, START_SCREEN_TILE_SMALL_HEIGHT,
        START_SCREEN_TILE_STYLE_SMALL},

    {"find_phone", "Find phone", NULL, NULL, &findphone_icon,
        0xD50055, 163, 162, START_SCREEN_TILE_SMALL_WIDTH, START_SCREEN_TILE_SMALL_HEIGHT,
        START_SCREEN_TILE_STYLE_SMALL},

    {"calendar", "Calendar", NULL, NULL, &calendar_icon,
        0xF28A12, 8, 216, START_SCREEN_TILE_LARGE_WIDTH, START_SCREEN_TILE_LARGE_HEIGHT,
        START_SCREEN_TILE_STYLE_LARGE},

    {"timer", "Timer", NULL, NULL, &timer_icon,
        0xF28A12, 163, 216, START_SCREEN_TILE_SMALL_WIDTH, START_SCREEN_TILE_SMALL_HEIGHT,
        START_SCREEN_TILE_STYLE_SMALL},

    {"stopwatch", "Stopwatch", NULL, NULL, &stopwatch_icon,
        0xF28A12, 163, 270, START_SCREEN_TILE_SMALL_WIDTH, START_SCREEN_TILE_SMALL_HEIGHT,
        START_SCREEN_TILE_STYLE_SMALL},

};

const uint16_t start_screen_app_count =
    sizeof(start_screen_apps) / sizeof(start_screen_apps[0]);
