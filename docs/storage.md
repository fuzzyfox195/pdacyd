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
