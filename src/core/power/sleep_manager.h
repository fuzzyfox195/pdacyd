#ifndef SLEEP_MANAGER_H
#define SLEEP_MANAGER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void sleep_manager_initialize(void);
bool sleep_manager_enter_light_sleep(void);
bool sleep_manager_enter_deep_sleep(void);
bool sleep_manager_woke_from_sleep_button(void);

#ifdef __cplusplus
}
#endif

#endif /* SLEEP_MANAGER_H */
