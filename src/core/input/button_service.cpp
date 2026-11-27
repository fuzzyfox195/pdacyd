#include "button_service.h"

#include "../../../include/lcd_pins.h"
#include "../../../include/input_pins.h"
#include "../ui_input_handler.h"

#include <Arduino.h>
#include <LovyanGFX.hpp>

namespace {
struct DebouncedButton {
    PhysicalButton button;
    uint8_t pin;
    bool is_direct_gpio;
    bool raw_pressed;
    bool stable_pressed;
    bool suppress_until_release;
    bool long_press_sent;
    uint32_t raw_changed_at;
    uint32_t pressed_at;
};

DebouncedButton buttons[] = {
    {PhysicalButton::BACK, BUTTON_BACK_PCF_BIT, false},
    {PhysicalButton::HOME, BUTTON_HOME_PCF_BIT, false},
    {PhysicalButton::OPTION, BUTTON_OPTION_PCF_BIT, false},
    {PhysicalButton::FAVORITE, BUTTON_FAVORITE_PCF_BIT, false},
    {PhysicalButton::SLEEP, SLEEP_BUTTON_GPIO, true},
};
uint32_t last_pcf_poll_ms;
bool pcf_ready;
bool pcf_sample_available;

bool writePcf8575(uint16_t value)
{
    auto transaction = lgfx::i2c::beginTransaction(
        TOUCH_I2C_PORT, PCF8575_I2C_ADDRESS, TOUCH_I2C_FREQ, false);
    if (transaction.has_error()) return false;
    const uint8_t bytes[2] = {
        static_cast<uint8_t>(value & 0xFF),
        static_cast<uint8_t>(value >> 8),
    };
    const auto write_result = lgfx::i2c::writeBytes(TOUCH_I2C_PORT, bytes, sizeof(bytes));
    const auto end_result = lgfx::i2c::endTransaction(TOUCH_I2C_PORT);
    return !write_result.has_error() && !end_result.has_error();
}

bool readPcf8575(uint16_t &value)
{
    auto transaction = lgfx::i2c::beginTransaction(
        TOUCH_I2C_PORT, PCF8575_I2C_ADDRESS, TOUCH_I2C_FREQ, true);
    if (transaction.has_error()) return false;
    uint8_t bytes[2] = {0xFF, 0xFF};
    const auto read_result = lgfx::i2c::readBytes(TOUCH_I2C_PORT, bytes, sizeof(bytes), true);
    const auto end_result = lgfx::i2c::endTransaction(TOUCH_I2C_PORT);
    if (read_result.has_error() || end_result.has_error()) return false;
    value = static_cast<uint16_t>(bytes[0]) |
            (static_cast<uint16_t>(bytes[1]) << 8);
    return true;
}

void emit(const DebouncedButton &button, ButtonEventType type)
{
    static const char *button_names[] = {"BACK", "HOME", "OPTION", "FAVORITE", "SLEEP"};
    static const char *event_names[] = {"PRESSED", "RELEASED", "SHORT_PRESS", "LONG_PRESS"};
    char line[96];
    snprintf(line, sizeof(line), "uptime_ms=%lu button=%s event=%s",
             static_cast<unsigned long>(millis()), button_names[static_cast<uint8_t>(button.button)],
             event_names[static_cast<uint8_t>(type)]);
    Serial.printf("[BUTTON] %s\n", line);

    const ButtonEvent event{button.button, type};
    ui_input_handler_handle_button_event(&event);
}

bool isPressedLevel(bool high_level)
{
    return BUTTON_ACTIVE_LOW ? !high_level : high_level;
}

void updateButton(DebouncedButton &button, bool raw_pressed, uint32_t now_ms)
{
    if (raw_pressed != button.raw_pressed) {
        button.raw_pressed = raw_pressed;
        button.raw_changed_at = now_ms;
    }

    if (button.raw_pressed != button.stable_pressed &&
        static_cast<uint32_t>(now_ms - button.raw_changed_at) >= BUTTON_DEBOUNCE_MS) {
        button.stable_pressed = button.raw_pressed;
        if (button.stable_pressed) {
            emit(button, ButtonEventType::PRESSED);
            button.pressed_at = now_ms;
        } else {
            emit(button, ButtonEventType::RELEASED);
            if (button.suppress_until_release) {
                button.suppress_until_release = false;
            } else if (!button.long_press_sent) {
                emit(button, ButtonEventType::SHORT_PRESS);
            }
            button.long_press_sent = false;
        }
    }

    if (button.stable_pressed && button.button == PhysicalButton::SLEEP &&
        !button.suppress_until_release && !button.long_press_sent &&
        static_cast<uint32_t>(now_ms - button.pressed_at) >= SLEEP_LONG_PRESS_MS) {
        button.long_press_sent = true;
        emit(button, ButtonEventType::LONG_PRESS);
    }
}
} // namespace

extern "C" bool button_service_init(void)
{
    pinMode(SLEEP_BUTTON_GPIO, INPUT_PULLUP);
    const bool sleep_pressed_at_boot = isPressedLevel(digitalRead(SLEEP_BUTTON_GPIO) == HIGH);
    const uint32_t now = millis();
    buttons[4].raw_pressed = sleep_pressed_at_boot;
    buttons[4].stable_pressed = sleep_pressed_at_boot;
    buttons[4].raw_changed_at = now;
    buttons[4].suppress_until_release = sleep_pressed_at_boot;

    const auto bus_result = lgfx::i2c::init(TOUCH_I2C_PORT, TOUCH_SDA, TOUCH_SCL);
    if (bus_result.has_error()) {
        Serial.println("[INPUT] I2C bus initialization failed; PCF8575 buttons unavailable");
        return false;
    }

    // PCF8575 pins are inputs only after their quasi-bidirectional latches are set HIGH.
    pcf_ready = writePcf8575(0xFFFF);
    if (!pcf_ready) {
        Serial.println("[INPUT] PCF8575 not found at 0x20; direct Sleep button remains available");
        return false;
    }

    uint16_t initial_state = 0xFFFF;
    pcf_sample_available = readPcf8575(initial_state);
    for (size_t i = 0; i < 4; ++i) {
        const bool pressed = isPressedLevel((initial_state & (1U << buttons[i].pin)) != 0);
        buttons[i].raw_pressed = pressed;
        buttons[i].stable_pressed = pressed;
        buttons[i].raw_changed_at = now;
        buttons[i].suppress_until_release = pressed;
    }
    Serial.println("[INPUT] PCF8575 initialized at 0x20; button pins released HIGH");
    return true;
}

extern "C" bool button_service_pcf8575_available(void)
{
    return pcf_ready && pcf_sample_available;
}

extern "C" uint16_t button_service_pcf8575_pressed_mask(void)
{
    uint16_t pressed_mask = 0;
    for (size_t i = 0; i < 4; ++i) {
        if (buttons[i].stable_pressed) pressed_mask |= static_cast<uint16_t>(1U << buttons[i].pin);
    }
    return pressed_mask;
}

extern "C" void button_service_poll(uint32_t now_ms)
{
    uint16_t pcf_state = 0xFFFF;
    bool have_pcf_sample = false;
    if (pcf_ready &&
        static_cast<uint32_t>(now_ms - last_pcf_poll_ms) >= BUTTON_POLL_INTERVAL_MS) {
        last_pcf_poll_ms = now_ms;
        have_pcf_sample = readPcf8575(pcf_state);
        pcf_sample_available = have_pcf_sample;
    }

    for (size_t i = 0; i < 5; ++i) {
        DebouncedButton &button = buttons[i];
        if (button.is_direct_gpio) {
            const bool pressed = isPressedLevel(digitalRead(SLEEP_BUTTON_GPIO) == HIGH);
            updateButton(button, pressed, now_ms);
            continue;
        }
        if (!have_pcf_sample) continue;
        const bool pressed = isPressedLevel((pcf_state & (1U << button.pin)) != 0);
        updateButton(button, pressed, now_ms);
    }
}
