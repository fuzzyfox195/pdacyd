#ifndef LOCK_SCREEN_H
#define LOCK_SCREEN_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void ui_lock_screen_screen_init(void);
void ui_lock_screen_set_resume_mode(bool resume_previous);

#ifdef __cplusplus
}
#endif

#endif // LOCK_SCREEN_H
