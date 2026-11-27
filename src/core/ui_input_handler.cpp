#include "ui_input_handler.h"

#include "application_manager/application_manager.h"
#include "display/display_manager.h"
#include "power/power_manager.h"
#include "screen_manager/screen_manager.h"
#include "../ui/screens/sleep_options/sleep_options.h"

extern "C" void ui_input_handler_handle_button_event(const ButtonEvent *event)
{
    if (event == nullptr || (event->type != ButtonEventType::SHORT_PRESS &&
        event->type != ButtonEventType::LONG_PRESS)) return;

    if (screen_manager_is_locked()) {
        if (event->button == PhysicalButton::HOME &&
            event->type == ButtonEventType::SHORT_PRESS) {
            if (display_manager_is_sleeping()) display_manager_wake();
            if (screen_manager_unlock()) {
                application_manager_resume_after_sleep();
            }
        } else if (event->button == PhysicalButton::SLEEP &&
                   event->type == ButtonEventType::LONG_PRESS) {
            power_manager_request_deep_sleep();
        }
        return;
    }

    if (event->button == PhysicalButton::SLEEP) {
        if (event->type == ButtonEventType::LONG_PRESS) {
            power_manager_request_deep_sleep();
        } else {
            (void)sleep_options_show();
        }
        return;
    }

    if (event->type != ButtonEventType::SHORT_PRESS) return;
    switch (event->button) {
    case PhysicalButton::BACK:
        if (screen_manager_has_modal()) sleep_options_close();
        else (void)application_manager_back();
        break;
    case PhysicalButton::HOME:
        if (screen_manager_has_modal()) sleep_options_close();
        application_manager_home();
        break;
    case PhysicalButton::OPTION:
        application_manager_option();
        break;
    case PhysicalButton::FAVORITE:
        application_manager_favorite();
        break;
    case PhysicalButton::SLEEP:
        break;
    }
}
