#include "display_manager.h"

#include "../../../include/LGFX_config.hpp"

extern LGFX lcd;

namespace {
bool display_sleeping;
}

extern "C" void display_manager_sleep(void)
{
    lcd.sleep();
    display_sleeping = true;
}

extern "C" void display_manager_wake(void)
{
    lcd.wakeup();
    display_sleeping = false;
}

extern "C" bool display_manager_is_sleeping(void)
{
    return display_sleeping;
}
