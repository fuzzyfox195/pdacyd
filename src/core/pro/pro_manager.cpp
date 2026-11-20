#include "pro_manager.h"
#include "../system/system_comm.h"
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Core 0 owns background services. This Milestone-01 task is a scaffold:
// Wi-Fi, SNTP and weather workers will be added behind this manager.
static constexpr BaseType_t PRO_CORE = 0;
static constexpr UBaseType_t PRO_PRIORITY = 2;
static constexpr uint32_t PRO_STACK_BYTES = 4096;

static void pro_task(void *) {
    SystemSnapshot state{};
    state.network = NetworkState::DISCONNECTED;
    system_snapshot_write(&state);
    SystemEvent started{SystemEventType::PRO_STARTED, 0, millis()};
    system_publish_event(&started);

    TickType_t last_wake = xTaskGetTickCount();
    for (;;) {
        SystemCommand command{};
        // Commands are serviced promptly, but the task also emits a low-rate heartbeat.
        if (system_receive_command(&command, pdMS_TO_TICKS(250))) {
            // Command handling is intentionally stubbed until service modules are introduced.
            SystemEvent ev{SystemEventType::PRO_HEARTBEAT,
                           static_cast<uint32_t>(command.type), millis()};
            system_publish_event(&ev);
        }

        state.uptime_ms = millis();
        system_snapshot_write(&state);
        SystemEvent heartbeat{SystemEventType::PRO_HEARTBEAT, state.uptime_ms, millis()};
        system_publish_event(&heartbeat);
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(2000));
    }
}

bool pro_manager_start(void) {
    return xTaskCreatePinnedToCore(pro_task, "pro_manager", PRO_STACK_BYTES,
                                  nullptr, PRO_PRIORITY, nullptr, PRO_CORE) == pdPASS;
}
