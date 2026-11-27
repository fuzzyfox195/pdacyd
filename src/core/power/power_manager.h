#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    POWER_MODEM_SLEEP,
    POWER_LIGHT_SLEEP,
    POWER_DEEP_SLEEP,
} power_sleep_mode_t;

bool power_manager_request_sleep(power_sleep_mode_t mode);
void power_manager_request_deep_sleep(void);

#ifdef __cplusplus
}
#endif

#endif /* POWER_MANAGER_H */
