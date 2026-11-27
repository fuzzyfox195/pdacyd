#include "sleep_manager.h"

#include "../display/display_manager.h"
#include "../../../include/input_pins.h"
#include "../../ui/main_ui.h"

#include <Arduino.h>
#include <driver/gpio.h>
#include <driver/rtc_io.h>
#include <esp_err.h>
#include <esp_sleep.h>

namespace {
bool sleep_button_wakeup;

bool prepareSleepButtonWake(void)
{
    pinMode(SLEEP_BUTTON_GPIO, INPUT_PULLUP);
    esp_err_t result = gpio_wakeup_enable(static_cast<gpio_num_t>(SLEEP_BUTTON_GPIO),
                                          GPIO_INTR_LOW_LEVEL);
    if (result == ESP_OK) result = esp_sleep_enable_gpio_wakeup();
    if (result != ESP_OK) {
        Serial.printf("[POWER] GPIO light-sleep wake setup failed: %s\n",
                      esp_err_to_name(result));
        return false;
    }
    return true;
}
} // namespace

extern "C" void sleep_manager_initialize(void)
{
    const esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    sleep_button_wakeup = cause == ESP_SLEEP_WAKEUP_EXT1;
    Serial.printf("[POWER] boot wake cause=%d%s\n", static_cast<int>(cause),
                  sleep_button_wakeup ? " (Sleep button)" : "");
    pinMode(SLEEP_BUTTON_GPIO, INPUT_PULLUP);
}

extern "C" bool sleep_manager_woke_from_sleep_button(void)
{
    return sleep_button_wakeup;
}

extern "C" bool sleep_manager_enter_light_sleep(void)
{
    if (!prepareSleepButtonWake()) return false;
    display_manager_sleep();
    delay(20);
    const esp_err_t result = esp_light_sleep_start();
    gpio_wakeup_disable(static_cast<gpio_num_t>(SLEEP_BUTTON_GPIO));
    display_manager_wake();
    if (result != ESP_OK) {
        Serial.printf("[POWER] light sleep failed: %s\n", esp_err_to_name(result));
        return false;
    }
    return true;
}

extern "C" bool sleep_manager_enter_deep_sleep(void)
{
    pinMode(SLEEP_BUTTON_GPIO, INPUT_PULLUP);
    const uint32_t release_wait_started = millis();
    while (digitalRead(SLEEP_BUTTON_GPIO) == LOW &&
           static_cast<uint32_t>(millis() - release_wait_started) < SLEEP_RELEASE_TIMEOUT_MS) {
        delay(10);
    }
    if (digitalRead(SLEEP_BUTTON_GPIO) == LOW) {
        Serial.println("[POWER] Sleep button stayed held; Deep sleep cancelled to prevent wake loop");
        return false;
    }
    esp_err_t result = rtc_gpio_init(static_cast<gpio_num_t>(SLEEP_BUTTON_GPIO));
    if (result == ESP_OK) {
        result = rtc_gpio_set_direction(static_cast<gpio_num_t>(SLEEP_BUTTON_GPIO),
                                        RTC_GPIO_MODE_INPUT_ONLY);
    }
    if (result == ESP_OK) {
        result = rtc_gpio_pullup_en(static_cast<gpio_num_t>(SLEEP_BUTTON_GPIO));
    }
    if (result == ESP_OK) {
        result = esp_sleep_enable_ext1_wakeup(1ULL << SLEEP_BUTTON_GPIO,
                                              ESP_EXT1_WAKEUP_ANY_LOW);
    }
    if (result != ESP_OK) {
        Serial.printf("[POWER] deep sleep wake setup failed: %s\n",
                      esp_err_to_name(result));
        return false;
    }

    display_manager_sleep();
    Serial.flush();
    esp_deep_sleep_start();
    return true;
}
