#pragma once

#include "time_types.h"

namespace PDACYD {

class TimeService {
public:
    bool begin();

    ClockSnapshot getSnapshot() const;

    bool isSynchronized() const;

    bool requestSync();

    // Called by the PRO task at a low rate; all service methods belong to that task.
    void poll(uint32_t now_ms, bool network_connected, bool initial_scan_complete);

    void setTimezone(const char* timezone);
};

}
