# PDACYD SD storage

Mount point: `/sdcard`. SD is persistent storage, not inter-core IPC. Core communication remains RAM queues/events/snapshots; durable records are written to SD when they change or after successful service updates.

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
- `/sdcard/data/clock.json`: last synced timestamp/timezone/status
- `/sdcard/data/weather.json`: last weather snapshot, source and timestamp
- `/sdcard/logs/system.log`, `error.log`: bounded text logs (rotation is a follow-up)

Writes use a temporary file and rename where possible. This reduces exposure to partial JSON writes but is not a full transactional guarantee against SD removal or power loss. Avoid frequent writes; write configs on change, clock only on meaningful sync/update, weather only after successful fetch, and logs in batches.

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
  "schema_version": 1,
  "known_networks": [
    { "ssid": "HomeWiFi", "password": "replace-me", "priority": 1 }
  ]
}
```
Do not commit real Wi-Fi credentials. Plain JSON stores secrets unencrypted.

### clock.json
```json
{
  "schema_version": 1,
  "last_sync_unix": 0,
  "timezone": "Asia/Ho_Chi_Minh",
  "last_sync_ok": false
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
