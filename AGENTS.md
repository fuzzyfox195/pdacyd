# Repository Guidelines

## Project Structure & Module Organization

This repository is Arduino firmware for an ESP32-S3 CYD using PlatformIO, LVGL 8, and LovyanGFX. `src/main.cpp` initializes the board, display DMA, touch input, and LVGL loop. `src/core/` contains lifecycle, screen management, queues, and background services. `src/apps/<app>/` holds app descriptors, UI, and assets; `src/apps/app_registry.h` is the ordered registry. Shared screens and themes are in `src/ui/`. Board pins and LVGL settings are in `include/lcd_pins.h` and `include/lv_conf.h`. Docs are in `docs/`; `test/` has guidance but no suite yet.

## Build, Test, and Development Commands

- `pio run -e esp32-s3-devkitc1-n16r8` builds the firmware for the configured board.
- `pio run -e esp32-s3-devkitc1-n16r8 -t upload` builds and uploads it to a connected board.
- `pio device monitor` opens the serial monitor at 115200 baud.
- `pio test -e esp32-s3-devkitc1-n16r8` runs PlatformIO tests when test cases have been added.

For UI or hardware changes, build first and verify on the target display and touch hardware when available. PSRAM is required by the current configuration.

## Coding Style & Naming Conventions

Follow existing C/C++ style: four-space indentation, braces on the same line for functions and control blocks, and short, focused functions. Use `snake_case` for functions and variables, `UPPER_SNAKE_CASE` for macros/constants, and `*_t` for typedef names. Keep app code inside its app directory and shared UI behavior under `src/ui/`. Add launchable apps to `APP_REGISTRY` once; avoid duplicating app metadata already held in its descriptor. No repository-wide formatter or linter is configured.

## Testing Guidelines

There is no established automated test suite or coverage threshold yet. Add PlatformIO tests under `test/` as appropriate, with names that describe the behavior under test. At minimum, run the firmware build after changes; test display, touch, DMA, and app lifecycle changes on-device when possible.

## Commit & Pull Request Guidelines

Recent commits use concise imperative or descriptive subjects (for example, `Add system log app` or `Enhance Wi-Fi signal monitoring and status display in the UI`). Keep commits focused and explain the user-visible change. Pull requests should summarize behavior and affected modules, list build or device checks performed, link related issues when applicable, and include screenshots or a short recording for UI changes.

## Configuration & Hardware Notes

Keep board-specific pins, rotation, resolution, and display settings in `include/lcd_pins.h`; keep LVGL feature and memory settings in `include/lv_conf.h`. Do not commit credentials, local SD-card configuration, build outputs, or device-specific secrets.
