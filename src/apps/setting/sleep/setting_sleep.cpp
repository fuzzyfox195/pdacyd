#include "setting_sleep.h"
#include "../../../ui/screens/sleep_options/sleep_options.h"

extern "C" void setting_sleep_create_ui(lv_obj_t *parent)
{
    (void)parent;
    (void)sleep_options_show();
}
