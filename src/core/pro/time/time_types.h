#pragma once

#include <stdint.h>
#include <time.h>

namespace PDACYD {

struct ClockSnapshot {
    time_t unix_time;
    bool synchronized;
    uint32_t last_sync_ms;
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t weekday;
    bool valid;
};

}
