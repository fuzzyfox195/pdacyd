#pragma once

#include <stdint.h>

namespace PDACYD {

struct SystemConfig {
    uint32_t schema_version;

    char device_name[32];
    char timezone[40];

    uint32_t sleep_timeout_seconds;
    uint8_t brightness;

    bool auto_connect_wifi;
};

struct UIConfig {
    uint32_t schema_version;

    char theme[16];
    char accent_color[16];

    bool show_status_bar;
    bool show_battery_percentage;
};

struct SystemConfigDefaults {
    static SystemConfig system();
    static UIConfig ui();
};

}