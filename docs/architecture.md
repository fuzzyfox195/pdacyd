# Firmware architecture

## Project

This is a PlatformIO project (`src_dir = src`) using Arduino on `esp32-s3-devkitc1-n16r8`. The UI is mostly C and uses LVGL 8.x. Board setup, LovyanGFX display/touch integration, DMA flushing, battery sampling, and the LVGL loop are in `src/main.cpp`.

## Module boundaries

- `src/core/application_manager/`: app descriptor contract and one-foreground-app lifecycle.
- `src/core/pro/`: Core 0 background services. `network/` reads SD Wi-Fi profiles and owns connection attempts; `time/` syncs from the Time.Now HTTPS API and owns the clock cache; `pro_logger` writes service events to SD. SNTP is used only to bootstrap TLS when no trusted clock cache exists.
- `src/core/system/`: fixed-size command/event queues and a mutex-protected cross-core snapshot.
- `src/core/screen_manager/`: LVGL screen selection, including return to the existing Home/Start Screen.
- `src/apps/`: the ordered app registry and one directory per app. Each app owns its descriptor, Start Screen metadata, icon, UI, and lifecycle callbacks.
- `src/ui/`: screens, status bar, and theme. The Start Screen reads app descriptors from the registry and forwards a tile's stable ID to the Application Manager.
- `src/main.cpp`: hardware initialization and the existing single LVGL execution loop.

```text
src/
├── core/
│   ├── application_manager/
│   ├── pro/network/           # SD Wi-Fi profile and connection service
│   ├── pro/time/              # Time.Now API clock and SD cache service
│   ├── screen_manager/
│   └── system/                # inter-core snapshots and events
├── apps/
│   ├── app_registry.h        # single ordered app list
│   ├── app_catalog.c         # lookup over that list
│   └── {weather,voice_note,music,setting,find_phone,calendar,timer,stopwatch}/
├── ui/                       # screens, status bar, and theme
└── main.cpp                  # board setup and LVGL owner loop
```

## App and Start Screen metadata

`src/apps/app_registry.h` is the one ordered registry used by both app lookup and Start Screen rendering. App display names, IDs, tile size, color, preview strings, and icon references live in each app's immutable descriptor. The icon source also lives in that app's directory. Strings, descriptors, registry entries, and icon data are static, so they do not require heap allocation for configuration.

The Start Screen uses the `.x` and `.y` coordinates in each app's descriptor and computes content height from the lowest tile. Tile dimensions, margins, and spacing remain shared UI rules; position changes stay inside the app's directory.

## Lifecycle

`ui_init()` initializes the manager once. Launch is accepted only when no app is active. The manager looks up the ID in `app_catalog.c`, invokes `init`, constructs the app screen, switches to it, then invokes `on_enter`. If initialization or screen construction fails, it calls `deinit` and leaves Home active. Back calls `application_manager_close()`: `on_exit`, switch to Home, destroy the app screen, then `deinit`. The Stopwatch app deletes its LVGL timer on exit. Shared services are not torn down by app exit.

LVGL calls happen from UI event callbacks, `ui_init()`, or `system_app_poll()` on the existing Arduino loop's LVGL owner. The PRO task tries Wi-Fi profiles from `/config/wifi.json` in priority order, performs time API and SD work on Core 0, then publishes clock fields through the system snapshot. If all profiles fail, it holds the fallback date at 2024-01-01 00:00. The UI loop updates every status bar and refreshes the Calendar tile when the date changes. Weather, music, and phone services remain future work.

Time source: [World Time API by Time.Now](https://time.now).
