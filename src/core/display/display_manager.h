#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void display_manager_sleep(void);
void display_manager_wake(void);
bool display_manager_is_sleeping(void);

#ifdef __cplusplus
}
#endif

#endif /* DISPLAY_MANAGER_H */
