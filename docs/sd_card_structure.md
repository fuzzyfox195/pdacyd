## Introduction
This guide outlines the directory structure and files on the SD card, designed to provide an AI agent with information about the files it will interact with in scenarios where it cannot directly access the SD card.

Please note that the directory structure and the files on the SD card are still under development.

### `config` folder
1. wifi.json

This file contains all information regarding known Wi-Fi networks, along with their connection priority order (from 1 to n).

``` text
{
  "connect_timeout_ms": 10000,
  "retry_per_network": 2,
  "networks": [
    {
      "ssid": "Wifi_name_01",
      "password": "password01",
      "priority": 1
    },
    {
      "ssid": "Wifi_name_02",
      "password": "password02",
      "priority": 2
    }
  ]
}
```
### `data` folder
1. clock.json
``` text
{
  "schema_version": 1,
  "timezone": "Asia/Ho_Chi_Minh",
  "last_sync_unix": 1795160700,
  "last_sync_ok": false,
  "last_sync_hour": 14,
  "last_sync_min": 45,
  "last_sync_sec": 0,
  "last_sync_day": 20,
  "last_sync_month": 11,
  "last_sync_year": 2026
}
```
2. weather.json

WIP

3. `/data/i2c.json` — danh sách thiết bị I2C đã nhận diện. `address` lưu dưới dạng chuỗi địa chỉ hex và `part` là mã linh kiện hiển thị trong Settings → I2C Scan.
``` text
{
  "i2c": [
    {
      "address": "0x20",
      "part": "INA219"
    },
    {
      "address": "0x3F",
      "part": "PCF8575"
    }
  ]
}
```
