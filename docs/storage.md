# PDACYD SD storage

Mount point: `/sdcard`. SD is persistent storage, not inter-core IPC. Core communication remains RAM queues/events/snapshots; durable records are written to SD by services.

## SDIO pins (4-bit)
- CLK: GPIO38
- CMD: GPIO40
- DAT0: GPIO39
- DAT1: GPIO41
- DAT2: GPIO48
- DAT3: GPIO47

## Files (JSON preferred)
- `/sdcard/config/system.json`: device preferences and system settings
- `/sdcard/config/wifi.json`: saved Wi-Fi profiles (passwords are plaintext; consider encryption or avoid storing secrets)
- `/sdcard/config/ui.json`: active theme and UI preferences
- `/sdcard/data/clock.json`: last API-synchronized timestamp/timezone/status, created with defaults when missing and refreshed once per minute after a successful sync
- `/sdcard/data/weather.json`: last weather snapshot, source and timestamp
- `/sdcard/logs/system.log`: system and PRO service log, cleared at every boot

Writes use a temporary file and rename where possible. This reduces exposure to partial JSON writes but is not a full transactional guarantee against SD removal or power loss. Configs are written on change, the clock cache once per minute after a successful sync, and service logs are appended to `system.log`. At startup the firmware removes the previous `system.log` and its rotated `.1` file, then creates a fresh log. Rotation at 96 KiB still bounds logs during a long boot session.

## JSON examples

### system.json
```json
{
  "schema_version": 1,
  "device_name": "PDACYD",
  "timezone": "Asia/Ho_Chi_Minh",
  "sleep_timeout_seconds": 60,
  "brightness": 80
}
```

### ui.json
```json
{
  "schema_version": 1,
  "theme": "dark",
  "accent": "blue"
}
```

### wifi.json
```json
{
  "connect_timeout_ms": 10000,
  "retry_per_network": 2,
  "networks": [
    { "ssid": "HomeWiFi", "password": "replace-me", "priority": 1 },
    { "ssid": "Phone hotspot", "password": "replace-me", "priority": 2 }
  ]
}
```
If the file is missing, the firmware creates it with an empty `networks` list. Do not commit real Wi-Fi credentials. Plain JSON stores secrets unencrypted.

### clock.json
```json
{
  "schema_version": 1,
  "last_sync_unix": 0,
  "timezone": "Asia/Ho_Chi_Minh",
  "last_sync_ok": false,
  "last_sync_hour": 0,
  "last_sync_min": 0,
  "last_sync_sec": 0,
  "last_sync_day": 0,
  "last_sync_month": 0,
  "last_sync_year": 0
}
```

### weather.json
```json
{
  "schema_version": 1,
  "provider": "",
  "location_id": "",
  "updated_unix": 0,
  "current": {
    "temperature_c": null,
    "feels_like_c": null,
    "humidity_percent": null,
    "condition_code": null
  }
}
```
