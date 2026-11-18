#ifndef APPLICATION_MANAGER_H
#define APPLICATION_MANAGER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void application_manager_init(void);
bool application_manager_launch(const char *app_id);
void application_manager_close(void);
const char *application_manager_current_id(void);

#ifdef __cplusplus
}
#endif
#endif
