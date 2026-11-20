#pragma once
#include "system_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#ifdef __cplusplus
extern "C" {
#endif

bool system_comm_init(void);
bool system_send_command(const SystemCommand *command, TickType_t wait_ticks);
bool system_receive_event(SystemEvent *event, TickType_t wait_ticks);
bool system_receive_command(SystemCommand *command, TickType_t wait_ticks);
void system_publish_event(const SystemEvent *event);
void system_snapshot_read(SystemSnapshot *out);
void system_snapshot_write(const SystemSnapshot *value);

#ifdef __cplusplus
}
#endif
