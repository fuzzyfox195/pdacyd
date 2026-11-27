#include "power_manager.h"

#include "sleep_manager.h"
#include "../application_manager/application_manager.h"

#include <Arduino.h>
#include <esp_err.h>
#include <esp_wifi.h>

extern "C" bool power_manager_request_sleep(power_sleep_mode_t mode)
{
    if (mode == POWER_MODEM_SLEEP) {
        const esp_err_t result = esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
        if (result != ESP_OK) {
            Serial.printf("[POWER] modem sleep failed: %s\n", esp_err_to_name(result));
            return false;
        }
        application_manager_prepare_sleep();
        return true;
    }

    application_manager_prepare_sleep();
    if (mode == POWER_LIGHT_SLEEP) {
        const bool slept = sleep_manager_enter_light_sleep();
        if (!slept) application_manager_resume_after_sleep();
        return slept;
    }
    if (mode == POWER_DEEP_SLEEP) {
        const bool entered = sleep_manager_enter_deep_sleep();
        if (!entered) application_manager_resume_after_sleep();
        return entered;
    }
    return false;
}

extern "C" void power_manager_request_deep_sleep(void)
{
    (void)power_manager_request_sleep(POWER_DEEP_SLEEP);
}
