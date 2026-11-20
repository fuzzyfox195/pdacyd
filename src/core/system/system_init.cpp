#include "system_init.h"
#include "system_comm.h"
#include "../storage/sd_storage.h"
#include "../pro/pro_manager.h"
#include <Arduino.h>

bool system_init(void) {
    const bool sd_ok = SDStorage::begin();
    Serial.printf("[SYS] SD storage: %s\\n", sd_ok ? "mounted" : "unavailable");
    if (!system_comm_init()) {
        Serial.println("[SYS] communication initialization failed");
        return false;
    }
    if (!pro_manager_start()) {
        Serial.println("[SYS] PRO task creation failed");
        return false;
    }
    Serial.println("[SYS] Dual-core scaffold started (PRO pinned to Core 0)");
    return true;
}

void system_app_poll(void) {
    SystemEvent event{};
    while (system_receive_event(&event, 0)) {
        // APP task (Arduino loop on Core 1) owns LVGL. Keep event processing here.
        if (event.type == SystemEventType::PRO_STARTED) {
            Serial.println("[APP] PRO started");
        }
    }
}
