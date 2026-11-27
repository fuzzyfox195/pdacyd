# Physical buttons and sleep

## Hardware mapping

The PDA reads four active-low buttons from a PCF8575 at I2C address `0x20` on the existing touch I2C bus (`TOUCH_SDA` GPIO16, `TOUCH_SCL` GPIO15): BACK=P17, HOME=P16, OPTION=P15, FAVORITE=P14. The PCF8575 service writes `0xFFFF` immediately after bus setup so all quasi-bidirectional pins are released HIGH before reading them. It samples the expander every 20 ms and debounces transitions for 35 ms.

The dedicated active-low Sleep button is ESP32-S3 GPIO14 with `INPUT_PULLUP`. This ESP32 pin is independent of PCF8575 P14. Its long-press threshold is 2500 ms. The pin assignments and timing values are centralized in `include/input_pins.h`.

## Input and navigation

`src/core/input/button_service.cpp` performs I2C/GPIO sampling in the Arduino UI loop; it does not use an interrupt handler or issue I2C transactions from an ISR. `src/core/ui_input_handler.cpp` maps button events centrally. BACK uses the app's `on_back` hook for an internal page, then closes to Home when the app has no deeper page. HOME closes the active app and displays the existing Start Screen. OPTION and FAVORITE dispatch to optional app callbacks; no app currently defines either action, so they safely do nothing. App screens no longer draw their own Back buttons.

Button transitions are written to Serial with uptime, button name, and event (`PRESSED`, `RELEASED`, `SHORT_PRESS`, or `LONG_PRESS`). Button handling does not write these events to the SD card.

## Sleep flow

A fresh boot, including a wake from Deep sleep, opens Lock Screen. HOME (PCF8575 P16) goes from this fresh-boot lock to Start Screen. A short Sleep press opens the full-screen Sleep Options screen. Settings → Sleep mode opens the same panel. Cancel and physical BACK return to the screen that was active before the panel opened. Available modes are:

- **Modem sleep:** asks the Wi-Fi driver to use `WIFI_PS_MIN_MODEM`, pauses the active app, places Lock Screen underneath, and turns the display off. HOME wakes the display, returns to the screen that was active before Sleep Options opened, and resumes the app.
- **Light sleep:** pauses the active app, sleeps the display/backlight, and enters ESP32 light sleep. GPIO14 LOW wakes it and shows Lock Screen. HOME returns to the screen that was active before Sleep Options opened, then resumes the app.
- **Deep sleep:** invokes active app `on_pause`, waits for GPIO14 to be released, configures GPIO14 as an RTC EXT1 `ANY_LOW` wake source, sleeps the display/backlight, then enters ESP32 deep sleep. A wake is a reboot; setup logs whether EXT1 caused it. Touchscreen input is not a sleep wake source.

Holding Sleep for 2.5 seconds requests Deep sleep directly. Waiting for button release before arming EXT1 avoids immediately waking from the same held press. If it remains LOW for 10 seconds, Deep sleep is cancelled to avoid a wake loop.

Modem and Light sleep keep the app screen and state in RAM until HOME resumes it; running Timer and Stopwatch counters pause during the lock interval. Deep sleep restarts into the fresh-boot Lock Screen and Start flow because app state is volatile. Physical validation is still required for the PCF8575 wiring, GPIO14 pull-up quality, button debounce, wake behavior, and current draw. An external current meter is needed for whole-board power comparisons.
