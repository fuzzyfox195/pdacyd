#include "app_catalog.h"
#include "weather/app_weather.h"
#include "voice_note/app_voice_note.h"
#include "music/app_music.h"
#include "setting/app_setting.h"
#include "find_phone/app_find_phone.h"
#include "calendar/app_calendar.h"
#include "timer/app_timer.h"
#include "stopwatch/app_stopwatch.h"
#include <string.h>

/* Keep the registry in the same stable order as the Start Screen catalog. */
static const app_descriptor_t *const app_registry[] = {
    &app_weather, &app_voice_note, &app_music, &app_setting,
    &app_find_phone, &app_calendar, &app_timer, &app_stopwatch,
};

const app_descriptor_t *app_catalog_find(const char *app_id)
{
    if (app_id == NULL) return NULL;
    for (size_t i = 0; i < app_catalog_count(); ++i) {
        if (strcmp(app_registry[i]->app_id, app_id) == 0) return app_registry[i];
    }
    return NULL;
}

size_t app_catalog_count(void)
{
    return sizeof(app_registry) / sizeof(app_registry[0]);
}

const app_descriptor_t *app_catalog_at(size_t index)
{
    return index < app_catalog_count() ? app_registry[index] : NULL;
}
