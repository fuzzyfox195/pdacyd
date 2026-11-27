#ifndef BUTTON_SERVICE_H
#define BUTTON_SERVICE_H

#include <stdint.h>

enum class PhysicalButton : uint8_t {
    BACK,
    HOME,
    OPTION,
    FAVORITE,
    SLEEP,
};

enum class ButtonEventType : uint8_t {
    PRESSED,
    RELEASED,
    SHORT_PRESS,
    LONG_PRESS,
};

struct ButtonEvent {
    PhysicalButton button;
    ButtonEventType type;
};

#ifdef __cplusplus
extern "C" {
#endif

bool button_service_init(void);
void button_service_poll(uint32_t now_ms);
bool button_service_pcf8575_available(void);
uint16_t button_service_pcf8575_pressed_mask(void);

#ifdef __cplusplus
}
#endif

#endif /* BUTTON_SERVICE_H */
