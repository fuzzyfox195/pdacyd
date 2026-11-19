#include "app_catalog.h"
#include "app_registry.h"

#include <string.h>

#define APP_REGISTRY_ENTRY(symbol) &symbol,
static const app_descriptor_t *const app_registry[] = {
    APP_REGISTRY(APP_REGISTRY_ENTRY)
};
#undef APP_REGISTRY_ENTRY

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
    return APP_REGISTRY_COUNT;
}

const app_descriptor_t *app_catalog_at(size_t index)
{
    return index < app_catalog_count() ? app_registry[index] : NULL;
}
