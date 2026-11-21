#ifndef APP_REGISTRY_H
#define APP_REGISTRY_H

#include "../core/application_manager/app_interface.h"

/* Add each launchable app here once. This is the single ordered app list used
 * by the Application Manager and Start Screen. */
#define APP_REGISTRY(X) \
    X(app_weather) \
    X(app_voice_note) \
    X(app_music) \
    X(app_setting) \
    X(app_find_phone) \
    X(app_calendar) \
    X(app_timer) \
    X(app_stopwatch) \
    X(app_system_log)

#define APP_REGISTRY_DECLARE(symbol) extern const app_descriptor_t symbol;
APP_REGISTRY(APP_REGISTRY_DECLARE)
#undef APP_REGISTRY_DECLARE

#define APP_REGISTRY_COUNT_ONE(symbol) + 1
enum { APP_REGISTRY_COUNT = 0 APP_REGISTRY(APP_REGISTRY_COUNT_ONE) };
#undef APP_REGISTRY_COUNT_ONE

#endif
