# Migration notes

## Changes

- Added `src/core/application_manager/` with an app descriptor and a one-app-at-a-time launcher/closer.
- Added `src/core/screen_manager/` to centralize foreground screen switching.
- Added `src/apps/app_catalog.c` as the single ID-to-descriptor registry.
- Added temporary per-app implementations for all eight original Start Screen IDs. Each app now creates its own titled placeholder screen, app-specific `Coming soon` message, and Back handler in its own source file.
- Removed `src/apps/common/placeholder_app.c` and `.h`; placeholder UI and no-op lifecycle callbacks are no longer shared across apps.
- Connected Start Screen card clicks to the Application Manager and initialized the manager as part of `ui_init()`.
- Updated the existing status bar's battery widget tracking so its ADC refresh updates all live screen instances and drops widgets when their screen is deleted.
- Added architecture and catalog documentation. Existing display, touch, ADC, theme, tile layout, icons, and PlatformIO configuration are unchanged.

## Build and validation

This is a PlatformIO Arduino project, not an ESP-IDF CMake project; no CMake files were added. Validation used the existing `esp32-s3-devkitc1-n16r8` PlatformIO environment. Hardware interaction and visual rendering require the board and are not established by compilation alone.

Build run: `pio run -e esp32-s3-devkitc1-n16r8` completed successfully after the per-app placeholder split. PlatformIO compiled all application and core files and linked `firmware.elf` and `firmware.bin`. The build reports 27,904 bytes RAM (8.5%) and 723,147 bytes flash (5.7%) for the configured limits. No on-device interaction test was run.

## Limitations

All catalog entries were nonfunctional static cards in the original code, so they currently use temporary UI skeletons. Clock/Wi-Fi/weather/music/phone/cache services and Home scheduling do not exist in the source and remain future work. The manager's lifecycle callbacks provide the extension point for real app resource cleanup. There are no app-owned timers, event subscriptions, buffers, or tasks in the temporary implementations.
