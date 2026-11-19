# PDACYD ESP32-S3 Smart Watch UI

PDACYD is an Arduino/PlatformIO firmware project that runs an LVGL-based touch interface on an ESP32-S3 smart-watch-style 320x240 display. It combines LovyanGFX display and touch drivers with a modular app catalog, lock/start screens, a status bar, themes, and custom app lifecycle management.

## Features

- LVGL 8 user interface with a 320x240 landscape display
- LovyanGFX display and capacitive touch integration
- DMA-assisted LVGL display flushing
- Lock screen and Start Screen navigation
- Battery level display driven by ADC sampling
- Central app registry and one-foreground-app lifecycle manager
- Light, dark, and sunset UI palettes
- Custom LVGL fonts
- Individual placeholder/UI implementations for the registered apps

## Hardware

The default configuration targets:

- Board: `ESP32-S3-DevKitC-1-N16R8`
- Display/controller: LCDWIKI 2.8-inch ES3C28P
- LCD: ILI9341, 320x240
- Touch controller: FT6236/FT6336U
- RAM: 16 MB flash, with PSRAM required

Hardware pin assignments and display/touch settings are defined in `include/lcd_pins.h`.

## App Catalog

The Start Screen is generated from `src/apps/app_registry.h`. The current registry contains:

| App ID | Display name |
|---|---|
| `weather` | Weather |
| `voice_note` | Voice note |
| `music` | Komposer |
| `setting` | Setting |
| `find_phone` | Find phone |
| `calendar` | Calendar |
| `timer` | Timer |
| `stopwatch` | Stopwatch |

The Stopwatch has a working timer UI. Other applications currently provide app-specific placeholder screens until their services and real behavior are implemented.

## Project Structure

```text
src/
├── main.cpp                         # Hardware, LVGL, display, touch, and ADC setup
├── core/
│   ├── application_manager/         # Launch, enter, exit, and close app lifecycle
│   └── screen_manager/             # Foreground LVGL screen switching
├── apps/
│   ├── app_registry.h              # Ordered app registry
│   ├── app_catalog.c               # App ID lookup
│   └── <app>/                       # App metadata, UI, icon, and lifecycle
└── ui/
    ├── main_ui.c                    # Top-level UI initialization
    ├── screens/                     # Lock and Start Screens
    ├── status_bar/                  # Status bar and battery display
    └── assets/                      # Themes and custom fonts

include/
├── lv_conf.h                        # LVGL configuration
└── lcd_pins.h                       # Board, display, and touch configuration

docs/
├── architecture.md                  # Firmware architecture
├── app_catalog.md                   # App registry overview
├── add_app_manual.md                # Instructions for adding an app
└── migration_notes.md               # Migration and validation notes
```

## Requirements

- Visual Studio Code with PlatformIO IDE, or the PlatformIO CLI
- Python 3
- USB data cable
- ESP32-S3 board with PSRAM
- Compatible ILI9341 display and FT6236/FT6336U touch controller

The project dependencies are managed by PlatformIO:

- LVGL `^8.3.11`
- LovyanGFX `^1.2.34`
- Adafruit NeoPixel
- Arduino ESP32 framework

## Build

Build the default environment:

```bash
pio run
```

Or select the explicit environment:

```bash
pio run -e esp32-s3-devkitc1-n16r8
```

If `pio` is not in `PATH`, use the PlatformIO executable installed with your PlatformIO environment:

```bash
~/.platformio/penv/bin/pio run
```

Build the firmware, then upload it:

```bash
pio run -e esp32-s3-devkitc1-n16r8 -t upload
```

Open the serial monitor:

```bash
pio device monitor
```

The serial monitor is configured for `115200` baud with ESP32 exception decoding.

## Adding an App

1. Create `src/apps/<app_id>/`.
2. Define an immutable `app_descriptor_t` with the app ID, display name, tile metadata, icon, and lifecycle callbacks.
3. Add the descriptor symbol to `APP_REGISTRY` in `src/apps/app_registry.h`.
4. Implement the app's UI and lifecycle as needed.

See `docs/add_app_manual.md` for a complete example. `APP_REGISTRY` is the only manual registration point used by the Application Manager and Start Screen.

## Custom Fonts

Custom LVGL font sources are stored in `src/ui/assets/font/`.

Fonts generated for a newer LVGL converter may include fields that are not present in LVGL 8.4. Check generated descriptor code for:

```c
.static_bitmap
LV_FONT_MONTSERRAT_<size>
&LV_FONT_MONTSERRAT_<size>
```

For this project:

- Remove `.static_bitmap = 0;` for LVGL 8 descriptors.
- Replace converter-generated fallback macro names with the actual LVGL font variable, for example:

```c
extern const lv_font_t lv_font_montserrat_48;
.fallback = &lv_font_montserrat_48,
```

- Enable the corresponding fallback font in `include/lv_conf.h`, for example:

```c
#define LV_FONT_MONTSERRAT_48 1
```

Declare custom font variables in `src/ui/assets/theme/ui_theme.h` with `LV_FONT_DECLARE(...)`, then reference them through theme macros or font pointers.

## Configuration

Key configuration files:

- `platformio.ini`: board, framework, dependencies, build flags, and partition settings
- `include/lv_conf.h`: LVGL features and built-in fonts
- `include/lcd_pins.h`: LCD/touch pins and display geometry
- `partitions.csv`: ESP32 flash partition layout
- `src/ui/assets/theme/ui_theme.h`: theme font and color macros

## Development Status

The repository provides the hardware integration and application-shell architecture. Real clock, Wi-Fi, weather, music, phone, and persistent storage services are not yet implemented. The current app screens are placeholders except for the Stopwatch UI.