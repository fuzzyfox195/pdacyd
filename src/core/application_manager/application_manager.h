#ifndef APPLICATION_MANAGER_H
#define APPLICATION_MANAGER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void application_manager_init(void);
bool application_manager_launch(const char *app_id);
void application_manager_close(void);
bool application_manager_back(void);
void application_manager_home(void);
void application_manager_option(void);
void application_manager_favorite(void);
void application_manager_prepare_sleep(void);
void application_manager_resume_after_sleep(void);
const char *application_manager_current_id(void);

#ifdef __cplusplus
}
#endif
#endif
