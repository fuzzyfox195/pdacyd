#ifndef SLEEP_OPTIONS_H
#define SLEEP_OPTIONS_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool sleep_options_show(void);
void sleep_options_close(void);
void sleep_options_lock_after_sleep(bool sleep_display);

#ifdef __cplusplus
}
#endif

#endif /* SLEEP_OPTIONS_H */
