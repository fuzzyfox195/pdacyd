# Status bar icons

The status bar uses LVGL primitives at runtime, so these SVGs are reference/source assets and do not require an SVG decoder in the firmware. They can also be exported for documentation or a future image-based UI.

- `phone_connect.svg`: phone outline and connection cue; currently a placeholder.
- `signal_strength.svg`: four ascending signal bars; currently a placeholder.
- `battery.svg`: battery outline with a 60% example fill; runtime fill is drawn by LVGL from the ADC reading.

Battery percentage currently maps raw 12-bit GPIO9 ADC values from 0 (empty) to 4095 (full), then rounds to the nearest 5%. Calibrate `adc_empty` and `adc_full` in `src/ui/status_bar/status_bar.c` to the actual battery voltage sensing circuit before relying on the displayed percentage.
