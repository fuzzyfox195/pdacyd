#include "system_init.h"
#include "system_comm.h"
#include "../storage/sd_storage.h"
#include "../storage/storage_config.h"
#include "../pro/pro_manager.h"
#include "../pro/pro_logger.h"
#include "../../ui/status_bar/status_bar.h"
#include "../../ui/screens/start_screen/start_screen.h"
#include <Arduino.h>
#include <esp_system.h>
#include <stdio.h>

bool system_init(void) {
    const esp_reset_reason_t reset_reason = esp_reset_reason();
    Serial.printf("[SYS] reset reason=%d\n", static_cast<int>(reset_reason));
    const bool sd_ok = SDStorage::begin();
    Serial.printf("[SYS] SD storage: %s\n", sd_ok ? "mounted" : "unavailable");
    if (sd_ok) {
        if (SDStorage::resetLog(SDStorage::SYSTEM_LOG)) {
            PDACYD::pro_log_info("SYSTEM", "boot reset reason=%d",
                                 static_cast<int>(reset_reason));
            PDACYD::pro_log_info("SYSTEM", "SD mounted; previous system logs cleared");
        } else {
            PDACYD::pro_logger_set_sd_enabled(false);
            Serial.println("[SYS] could not reset system log; SD logging disabled for this boot");
        }
    } else {
        PDACYD::pro_log_error("SYSTEM", "SD storage unavailable");
    }
    if (!system_comm_init()) {
        Serial.println("[SYS] communication initialization failed");
        return false;
    }
    if (!pro_manager_start()) {
        Serial.println("[SYS] PRO task creation failed");
        PDACYD::pro_log_error("SYSTEM", "PRO task creation failed");
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

    // Snapshot reads and all LVGL updates stay on the Arduino/UI core.
    SystemSnapshot state{};
    system_snapshot_read(&state);
    static bool displayed_wifi_initialized = false;
    static NetworkState displayed_network = NetworkState::UNINITIALIZED;
    static uint8_t displayed_signal_level = 0;
    if (!displayed_wifi_initialized || displayed_network != state.network ||
        displayed_signal_level != state.wifi_signal_level) {
        displayed_wifi_initialized = true;
        displayed_network = state.network;
        displayed_signal_level = state.wifi_signal_level;
        ui_status_bar_set_wifi_status(
            state.network == NetworkState::CONNECTED,
            state.network == NetworkState::ERROR,
            state.wifi_signal_level);
    }

    static uint8_t displayed_battery_percent = 255;
    if (displayed_battery_percent != state.battery_percent) {
        displayed_battery_percent = state.battery_percent;
        ui_status_bar_set_battery_percent(state.battery_percent);
    }

    static int displayed_hour = -1;
    static int displayed_minute = -1;
    static bool displayed_valid = false;
    const bool clock_valid = state.unix_time > 0;
    if (displayed_hour != state.time_hour || displayed_minute != state.time_minute ||
        displayed_valid != clock_valid) {
        displayed_hour = state.time_hour;
        displayed_minute = state.time_minute;
        displayed_valid = clock_valid;
        ui_status_bar_set_time(state.time_hour, state.time_minute, clock_valid);
    }

    static uint16_t displayed_year;
    static uint8_t displayed_month;
    static uint8_t displayed_day;
    static bool displayed_date_valid;
    if (displayed_year != state.time_year || displayed_month != state.time_month ||
        displayed_day != state.time_day || displayed_date_valid != clock_valid) {
        displayed_year = state.time_year;
        displayed_month = state.time_month;
        displayed_day = state.time_day;
        displayed_date_valid = clock_valid;

        static const char *month_names[] = {
            "Jan", "Feb", "Mar", "Apr", "May", "Jun",
            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
        };
        static const char *weekday_names[] = {
            "Sunday", "Monday", "Tuesday", "Wednesday",
            "Thursday", "Friday", "Saturday"
        };
        char title[20];
        char content[24];
        if (clock_valid && state.time_month >= 1 && state.time_month <= 12 &&
            state.time_weekday < 7) {
            snprintf(title, sizeof(title), "%u %s", state.time_day,
                     month_names[state.time_month - 1]);
            snprintf(content, sizeof(content), "%s %u",
                     weekday_names[state.time_weekday], state.time_year);
        } else {
            snprintf(title, sizeof(title), "-- ---");
            snprintf(content, sizeof(content), "Date unavailable");
        }
        ui_start_screen_update_tile("calendar", title, content);
    }
}
