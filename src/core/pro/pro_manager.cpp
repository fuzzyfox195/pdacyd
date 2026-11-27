#include "pro_manager.h"
#include "../system/system_comm.h"
#include "network/network_service.h"
#include "battery/battery_service.h"
#include "pro_logger.h"
#include "time/time_service.h"
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Core 0 owns background services; UI and LVGL remain on the Arduino loop.
static constexpr BaseType_t PRO_CORE = 0;
static constexpr UBaseType_t PRO_PRIORITY = 2;
// HTTPS/TLS handshake plus SD logging have a much higher transient stack
// requirement than the normal 250 ms service loop.
static constexpr uint32_t PRO_STACK_BYTES = 16 * 1024;
static PDACYD::TimeService time_service;

static void pro_task(void *) {
    SystemSnapshot state{};
    state.network = NetworkState::UNINITIALIZED;
    state.wifi_signal_level = 0;
    state.battery_percent = 0;
    PDACYD::pro_log_info("PRO", "task started core=%d", PRO_CORE);
    time_service.begin();
    PDACYD::NetworkService::begin();
    PDACYD::BatteryService::begin();
    system_snapshot_write(&state);
    SystemEvent started{SystemEventType::PRO_STARTED, 0, millis()};
    system_publish_event(&started);

    TickType_t last_wake = xTaskGetTickCount();
    uint32_t last_heartbeat_ms = millis();
    bool announced_time_sync = false;
    uint32_t last_battery_sample_ms = 0;
    for (;;) {
        const uint32_t now_ms = millis();
        if (last_battery_sample_ms == 0 ||
            static_cast<uint32_t>(now_ms - last_battery_sample_ms) >= 1000) {
            last_battery_sample_ms = now_ms;
            state.battery_percent = PDACYD::BatteryService::readPercent();
        }
        SystemCommand command{};
        while (system_receive_command(&command, 0)) {
            if (command.type == SystemCommandType::TIME_SYNC) {
                time_service.requestSync();
            }
        }

        PDACYD::NetworkService::poll(now_ms);
        const bool wifi_connected = PDACYD::NetworkService::isConnected();
        time_service.poll(now_ms, wifi_connected,
                          PDACYD::NetworkService::initialScanComplete());
        const PDACYD::ClockSnapshot clock = time_service.getSnapshot();
        const NetworkState network = PDACYD::NetworkService::getState();
        if (state.network != network) {
            state.network = network;
            SystemEvent network_event{SystemEventType::NETWORK_STATE_CHANGED,
                                      static_cast<uint32_t>(network), now_ms};
            system_publish_event(&network_event);
        }
        state.wifi_signal_level = PDACYD::NetworkService::signalLevel();
        state.uptime_ms = now_ms;
        state.time_synced = clock.synchronized;
        state.unix_time = static_cast<int64_t>(clock.unix_time);
        state.time_year = clock.year;
        state.time_month = clock.month;
        state.time_day = clock.day;
        state.time_hour = clock.hour;
        state.time_minute = clock.minute;
        state.time_second = clock.second;
        state.time_weekday = clock.weekday;
        system_snapshot_write(&state);
        if (clock.synchronized && !announced_time_sync) {
            announced_time_sync = true;
            SystemEvent synced{SystemEventType::TIME_SYNCED,
                               static_cast<uint32_t>(clock.unix_time), now_ms};
            system_publish_event(&synced);
        }
        if (static_cast<uint32_t>(now_ms - last_heartbeat_ms) >= 2000) {
            last_heartbeat_ms = now_ms;
            SystemEvent heartbeat{SystemEventType::PRO_HEARTBEAT, state.uptime_ms, now_ms};
            system_publish_event(&heartbeat);
        }
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(250));
    }
}

bool pro_manager_start(void) {
    return xTaskCreatePinnedToCore(pro_task, "pro_manager", PRO_STACK_BYTES,
                                  nullptr, PRO_PRIORITY, nullptr, PRO_CORE) == pdPASS;
}
