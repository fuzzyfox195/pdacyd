#ifndef APP_CATALOG_H
#define APP_CATALOG_H
#include "../core/application_manager/app_interface.h"
#include "app_registry.h"
#include <stddef.h>
const app_descriptor_t *app_catalog_find(const char *app_id);
size_t app_catalog_count(void);
const app_descriptor_t *app_catalog_at(size_t index);
#endif
