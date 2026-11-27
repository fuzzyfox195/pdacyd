#ifndef UI_INPUT_HANDLER_H
#define UI_INPUT_HANDLER_H

#include "input/button_service.h"

#ifdef __cplusplus
extern "C" {
#endif

void ui_input_handler_handle_button_event(const ButtonEvent *event);

#ifdef __cplusplus
}
#endif

#endif /* UI_INPUT_HANDLER_H */
