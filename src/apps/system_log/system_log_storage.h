#ifndef APP_SYSTEM_LOG_STORAGE_H
#define APP_SYSTEM_LOG_STORAGE_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool app_system_log_read_tail(char *buffer, size_t capacity);

#ifdef __cplusplus
}
#endif

#endif
