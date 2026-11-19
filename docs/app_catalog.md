# Start Screen app registry

`src/apps/app_registry.h` defines the ordered list of launchable apps. Each app descriptor in `src/apps/<app>/` owns its display name, icon, color, tile size, position, and optional preview text. The Start Screen uses the configured `x` and `y` values; there is no separate UI catalog.

When adding an app, add its files and icon under `src/apps/<app>/`, define its immutable descriptor and `.start_tile` coordinates there, then add its symbol once to `APP_REGISTRY` in `app_registry.h`. This is the only central app registration point; no per-app configuration under `src/ui/` is needed.
