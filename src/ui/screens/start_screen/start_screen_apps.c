#include "start_screen_apps.h"

/* Add an app by appending one entry here. Use a unique stable id; x/y positions
 * are relative to the scroll area, and width/height may use a standard size or
 * a custom size. LARGE tiles show title/content; SMALL tiles show app_name only.
 * Keep 8 px between cards and at the horizontal edges when following the demo. */

 /* For calculating the position of each tile 
    let the first tile be at (8, 0) 
    Large tile have size of 147x100
    so the neext tile by vertial will be at (8+147+8=163, 0)
    and the next tile by horizontal will be at (8, 0+100+8=116)
    note that we have large tiles and small tiles, so be careful with the positioning */

const start_screen_tile_config_t start_screen_apps[] = {

    {"weather", "Weather", "Ha Noi", "32 °C, feel like 34 °C",
        0x2583E8, 8, 0, START_SCREEN_TILE_LARGE_WIDTH, START_SCREEN_TILE_LARGE_HEIGHT,
        START_SCREEN_TILE_STYLE_LARGE},

    {"voice_note", "Voice note", "12 notes", "Use your voice to take note",
        0x159A79, 163, 0, START_SCREEN_TILE_LARGE_WIDTH, START_SCREEN_TILE_LARGE_HEIGHT,
        START_SCREEN_TILE_STYLE_LARGE},

    {"music", "Komposer", "Local", "Em dau\nPhung Khan Linh...",
        0x12B85A, 8, 108, START_SCREEN_TILE_LARGE_WIDTH, START_SCREEN_TILE_LARGE_HEIGHT,
        START_SCREEN_TILE_STYLE_LARGE},

    {"setting", "Setting", NULL, NULL,
        0x444444, 163, 108, START_SCREEN_TILE_SMALL_WIDTH, START_SCREEN_TILE_SMALL_HEIGHT,
        START_SCREEN_TILE_STYLE_SMALL},

    {"find_phone", "Find phone", NULL, NULL,
        0xD50055, 163, 162, START_SCREEN_TILE_SMALL_WIDTH, START_SCREEN_TILE_SMALL_HEIGHT,
        START_SCREEN_TILE_STYLE_SMALL},

    {"calendar", "Calendar", NULL, NULL,
        0xF28A12, 8, 216, START_SCREEN_TILE_LARGE_WIDTH, START_SCREEN_TILE_LARGE_HEIGHT,
        START_SCREEN_TILE_STYLE_LARGE},

    {"timer", "Timer", NULL, NULL,
        0xF28A12, 163, 216, START_SCREEN_TILE_SMALL_WIDTH, START_SCREEN_TILE_SMALL_HEIGHT,
        START_SCREEN_TILE_STYLE_SMALL},

    {"stopwatch", "Stopwatch", NULL, NULL,
        0xF28A12, 163, 270, START_SCREEN_TILE_SMALL_WIDTH, START_SCREEN_TILE_SMALL_HEIGHT,
        START_SCREEN_TILE_STYLE_SMALL},

};

const uint16_t start_screen_app_count =
    sizeof(start_screen_apps) / sizeof(start_screen_apps[0]);
