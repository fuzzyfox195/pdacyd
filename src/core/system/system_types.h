#pragma once
#include <stdint.h>

enum class NetworkState : uint8_t {
    UNINITIALIZED, DISCONNECTED, CONNECTING, CONNECTED, RECONNECTING, ERROR
};

enum class SystemEventType : uint8_t {
    PRO_STARTED, PRO_HEARTBEAT, NETWORK_STATE_CHANGED,
    WEATHER_UPDATED, WEATHER_FAILED, TIME_SYNCED, SYSTEM_ERROR
};

enum class SystemCommandType : uint8_t {
    WIFI_CONNECT, WIFI_DISCONNECT, WEATHER_REFRESH, TIME_SYNC
};

struct SystemEvent {
    SystemEventType type;
    uint32_t value;
    uint32_t timestamp_ms;
};

struct SystemCommand {
    SystemCommandType type;
    uint32_t value;
};

struct SystemSnapshot {
    NetworkState network;
    uint32_t uptime_ms;
    bool time_synced;
    int64_t unix_time;
    float temperature_c;
    uint8_t humidity;
    uint32_t weather_updated_ms;
    bool weather_valid;
};
