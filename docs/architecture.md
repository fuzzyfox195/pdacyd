# Firmware architecture

## Existing project

This repository is a PlatformIO project (`src_dir = src`) using the Arduino framework on `esp32-s3-devkitc1-n16r8`. UI code is mostly C and uses LVGL 8.x; `platformio.ini` requests `lvgl/lvgl@^8.3.11`, which resolved to 8.4.0 in the validated build. Board setup, LovyanGFX display/touch integration, DMA flushing, battery ADC sampling, and the LVGL timer loop are in `src/main.cpp`. PlatformIO compiles source files under `src/`; there is no ESP-IDF component CMake configuration in this checkout.

The UI was split into lock screen, Start Screen, status bar, and theme files. `src/ui/screens/start_screen/start_screen_apps.c` owns the Start Screen tile catalog. Its cards previously had no click handlers. No app implementations or app-specific lifecycle code existed elsewhere in `src/`.

## Current module boundaries

- `src/core/application_manager/`: app descriptor contract and one-foreground-app lifecycle.
- `src/core/screen_manager/`: LVGL screen loading, including return to the existing Home/Start Screen.
- `src/apps/`: catalog and one app directory per launchable catalog entry. Each app owns its placeholder UI and lifecycle callbacks.
- `src/ui/`: existing screens, status bar, theme, and icons. The Start Screen forwards a tile's stable ID to the Application Manager; it does not know app implementation symbols.
- `src/main.cpp`: retains hardware initialization and the single LVGL execution loop. No app or service task is introduced.

```text
src/
├── core/
│   ├── application_manager/  # descriptor contract and lifecycle
│   └── screen_manager/       # foreground LVGL screen selection
├── apps/
│   ├── app_catalog.c         # one ID registry
│   └── {weather,voice_note,music,setting,find_phone,calendar,timer,stopwatch}/
├── ui/                       # existing UI, assets, theme, status bar
└── main.cpp                  # board setup and LVGL owner loop
```

## Lifecycle

`ui_init()` initializes the manager once. Launch is accepted only when no app is active. The manager looks up the ID in `app_catalog.c`, invokes `init`, constructs the app screen, switches to it, then invokes `on_enter`. If initialization or screen construction fails, it calls `deinit` and leaves Home active. Back calls `application_manager_close()`: `on_exit`, switch to Home, destroy the app screen, then `deinit`. The currently provided placeholder apps hold no app-owned resources. Shared services are not torn down by app exit.

LVGL calls happen from UI event callbacks or `ui_init()` on the existing Arduino loop's LVGL owner. No background tasks are added. There is no Home Scheduler or asynchronous service implementation in the original project, so none is fabricated here; those services can be introduced when the firmware needs their behavior.

## Services and future work

The repository currently has no clock, Wi-Fi, weather, music, phone, or persistent cache service interfaces. Weather and music tile text remains the existing static preview content. App placeholders do not make network requests or create duplicate services. When services are added, they should own acquisition/cache state and communicate with the UI on the LVGL owner context.
