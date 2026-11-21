#include "system_comm.h"
#include <string.h>

static QueueHandle_t command_queue;
static QueueHandle_t event_queue;
static SemaphoreHandle_t snapshot_mutex;
static SystemSnapshot snapshot_data{};

bool system_comm_init(void) {
    if (command_queue || event_queue || snapshot_mutex) return false;
    command_queue = xQueueCreate(8, sizeof(SystemCommand));
    event_queue = xQueueCreate(16, sizeof(SystemEvent));
    snapshot_mutex = xSemaphoreCreateMutex();
    if (!command_queue || !event_queue || !snapshot_mutex) return false;
    snapshot_data.network = NetworkState::UNINITIALIZED;
    return true;
}

bool system_send_command(const SystemCommand *command, TickType_t wait_ticks) {
    if (!command_queue || !command) return false;
    return xQueueSend(command_queue, command, wait_ticks) == pdTRUE;
}

bool system_receive_command(SystemCommand *command, TickType_t wait_ticks) {
    if (!command_queue || !command) return false;
    return xQueueReceive(command_queue, command, wait_ticks) == pdTRUE;
}

bool system_receive_event(SystemEvent *event, TickType_t wait_ticks) {
    if (!event_queue || !event) return false;
    return xQueueReceive(event_queue, event, wait_ticks) == pdTRUE;
}

void system_publish_event(const SystemEvent *event) {
    if (!event_queue || !event) return;
    // Drop oldest event if the queue is full; events are hints, snapshots hold latest data.
    if (xQueueSend(event_queue, event, 0) != pdTRUE) {
        SystemEvent discarded{};
        xQueueReceive(event_queue, &discarded, 0);
        xQueueSend(event_queue, event, 0);
    }
}

void system_snapshot_read(SystemSnapshot *out) {
    if (!out || !snapshot_mutex) return;
    if (xSemaphoreTake(snapshot_mutex, portMAX_DELAY) == pdTRUE) {
        *out = snapshot_data;
        xSemaphoreGive(snapshot_mutex);
    }
}

void system_snapshot_write(const SystemSnapshot *value) {
    if (!value || !snapshot_mutex) return;
    if (xSemaphoreTake(snapshot_mutex, portMAX_DELAY) == pdTRUE) {
        snapshot_data = *value;
        xSemaphoreGive(snapshot_mutex);
    }
}
