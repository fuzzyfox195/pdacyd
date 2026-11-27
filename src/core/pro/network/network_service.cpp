#include "network_service.h"

#include "../pro_logger.h"
#include "../../storage/sd_storage.h"
#include "../../storage/storage_config.h"
#include <Arduino.h>
#include <WiFi.h>
#include <stdlib.h>
#include <string.h>

namespace PDACYD {
namespace NetworkService {
namespace {
constexpr size_t MAX_PROFILES = 8;
constexpr size_t SSID_CAPACITY = 33;
constexpr size_t PASSWORD_CAPACITY = 65;
constexpr uint32_t RSSI_POLL_INTERVAL_MS = 10000;

struct WifiProfile {
    char ssid[SSID_CAPACITY];
    char password[PASSWORD_CAPACITY];
    uint8_t priority;
};

enum class State : uint8_t { IDLE, CONNECTING, CONNECTED, EXHAUSTED };

WifiProfile profiles[MAX_PROFILES]{};
size_t profile_count;
size_t active_profile;
uint8_t attempts_started;
uint8_t retries_per_profile = 2;
uint32_t connect_timeout_ms = 10000;
uint32_t attempt_started_ms;
uint32_t last_rssi_sample_ms;
State state = State::IDLE;
bool initial_scan_complete;
bool has_connected_once;
bool has_rssi_sample;
uint8_t signal_level;

bool readNumber(const String &json, const char *key, int64_t &value) {
    const String needle = String("\"") + key + "\"";
    int position = json.indexOf(needle);
    if (position < 0) return false;
    position = json.indexOf(':', position + needle.length());
    if (position < 0) return false;
    const char *start = json.c_str() + position + 1;
    while (*start == ' ' || *start == '\t' || *start == '\r' || *start == '\n') ++start;
    char *end = nullptr;
    const long long parsed = strtoll(start, &end, 10);
    if (end == start) return false;
    value = static_cast<int64_t>(parsed);
    return true;
}

bool readString(const String &json, const char *key, char *out, size_t capacity) {
    if (out == nullptr || capacity == 0) return false;
    const String needle = String("\"") + key + "\"";
    int position = json.indexOf(needle);
    if (position < 0) return false;
    position = json.indexOf(':', position + needle.length());
    if (position < 0) return false;
    ++position;
    while (position < json.length() && (json[position] == ' ' || json[position] == '\t' ||
           json[position] == '\r' || json[position] == '\n')) ++position;
    if (position >= json.length() || json[position++] != '"') return false;

    size_t written = 0;
    while (position < json.length()) {
        char ch = json[position++];
        if (ch == '"') {
            out[written] = '\0';
            return true;
        }
        if (ch == '\\') {
            if (position >= json.length()) return false;
            ch = json[position++];
            if (ch == 'n') ch = '\n';
            else if (ch == 'r') ch = '\r';
            else if (ch == 't') ch = '\t';
        }
        if (written + 1 >= capacity) return false;
        out[written++] = ch;
    }
    return false;
}

bool loadProfiles() {
    if (!SDStorage::isMounted() || !SDStorage::ensureDirectory("/config")) {
        pro_log_error("NETWORK", "SD unavailable; cannot read wifi.json");
        return false;
    }

    const String template_json =
        "{\n"
        "  \"connect_timeout_ms\": 10000,\n"
        "  \"retry_per_network\": 2,\n"
        "  \"networks\": []\n"
        "}\n";
    if (!SDStorage::ensureJsonFile(SDStorage::WIFI_CONFIG, template_json)) {
        pro_log_error("NETWORK", "could not create /config/wifi.json");
        return false;
    }

    String json;
    if (!SDStorage::readJson(SDStorage::WIFI_CONFIG, json)) {
        pro_log_error("NETWORK", "could not read /config/wifi.json");
        return false;
    }
    pro_log_info("NETWORK", "read /config/wifi.json bytes=%u",
                 static_cast<unsigned>(json.length()));

    int64_t value = 0;
    if (readNumber(json, "connect_timeout_ms", value) &&
        value >= 1000 && value <= 60000) {
        connect_timeout_ms = static_cast<uint32_t>(value);
    }
    if (readNumber(json, "retry_per_network", value) && value >= 0 && value <= 5) {
        retries_per_profile = static_cast<uint8_t>(value);
    }

    const int networks_key = json.indexOf("\"networks\"");
    const int array_start = networks_key >= 0 ? json.indexOf('[', networks_key) : -1;
    if (array_start < 0) {
        pro_log_error("NETWORK", "wifi.json has no networks array");
        return false;
    }

    int cursor = array_start;
    while (profile_count < MAX_PROFILES) {
        const int object_start = json.indexOf('{', cursor + 1);
        const int array_end = json.indexOf(']', cursor + 1);
        if (object_start < 0 || (array_end >= 0 && array_end < object_start)) break;
        const int object_end = json.indexOf('}', object_start + 1);
        if (object_end < 0) break;

        const String object = json.substring(object_start, object_end + 1);
        WifiProfile profile{};
        int64_t priority = static_cast<int64_t>(profile_count + 1);
        const bool has_ssid = readString(object, "ssid", profile.ssid,
                                         sizeof(profile.ssid));
        const bool has_password = readString(object, "password", profile.password,
                                             sizeof(profile.password));
        (void)readNumber(object, "priority", priority);
        if (has_ssid && has_password && profile.ssid[0] != '\0') {
            if (priority < 0) priority = 0;
            if (priority > 255) priority = 255;
            profile.priority = static_cast<uint8_t>(priority);

            size_t insert_at = profile_count;
            while (insert_at > 0 &&
                   profiles[insert_at - 1].priority > profile.priority) {
                profiles[insert_at] = profiles[insert_at - 1];
                --insert_at;
            }
            profiles[insert_at] = profile;
            ++profile_count;
        }
        cursor = object_end;
    }

    pro_log_info("NETWORK", "loaded %u Wi-Fi profile(s), timeout=%lu ms retries=%u",
                 static_cast<unsigned>(profile_count),
                 static_cast<unsigned long>(connect_timeout_ms),
                 static_cast<unsigned>(retries_per_profile));
    return true;
}

void startAttempt(uint32_t now_ms) {
    if (active_profile >= profile_count) {
        state = State::EXHAUSTED;
        initial_scan_complete = true;
        return;
    }
    WiFi.disconnect(false, false);
    WiFi.begin(profiles[active_profile].ssid, profiles[active_profile].password);
    ++attempts_started;
    attempt_started_ms = now_ms;
    state = State::CONNECTING;
    initial_scan_complete = false;
    pro_log_info("NETWORK", "connecting profile=%u/%u ssid=\"%s\" attempt=%u",
                 static_cast<unsigned>(active_profile + 1),
                 static_cast<unsigned>(profile_count), profiles[active_profile].ssid,
                 static_cast<unsigned>(attempts_started));
}

void exhaustProfiles() {
    WiFi.disconnect(false, false);
    state = State::EXHAUSTED;
    initial_scan_complete = true;
    signal_level = 0;
    pro_log_error("NETWORK", "all %u configured Wi-Fi profile(s) failed",
                  static_cast<unsigned>(profile_count));
}

uint8_t levelFromRssi(int rssi) {
    if (rssi >= -55) return 4;
    if (rssi >= -67) return 3;
    if (rssi >= -75) return 2;
    return 1;
}

void updateSignalLevel(uint32_t now_ms) {
    if (!has_rssi_sample ||
        static_cast<uint32_t>(now_ms - last_rssi_sample_ms) >= RSSI_POLL_INTERVAL_MS) {
        last_rssi_sample_ms = now_ms;
        has_rssi_sample = true;
        const int rssi = WiFi.RSSI();
        signal_level = rssi == 0 ? 1 : levelFromRssi(rssi);
        Serial.printf("[NETWORK] signal quality rssi=%d dBm level=%u/4\n",
                      rssi, static_cast<unsigned>(signal_level));
    }
}
} // namespace

bool begin() {
    pro_log_info("NETWORK", "network service started on PRO core");
    profile_count = 0;
    if (!loadProfiles()) {
        initial_scan_complete = true;
        state = State::EXHAUSTED;
        return false;
    }

    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(false);
    WiFi.disconnect(false, false);
    active_profile = 0;
    attempts_started = 0;
    has_connected_once = false;
    has_rssi_sample = false;
    signal_level = 0;
    initial_scan_complete = profile_count == 0;
    if (profile_count == 0) {
        state = State::EXHAUSTED;
        pro_log_warn("NETWORK", "wifi.json contains no usable networks");
        return true;
    }

    startAttempt(millis());
    return true;
}

void poll(uint32_t now_ms) {
    if (state == State::CONNECTED) {
        if (WiFi.status() == WL_CONNECTED) {
            updateSignalLevel(now_ms);
            return;
        }
        pro_log_warn("NETWORK", "Wi-Fi connection lost; restarting profile list");
        signal_level = 0;
        has_rssi_sample = false;
        active_profile = 0;
        attempts_started = 0;
        startAttempt(now_ms);
        return;
    }
    if (state != State::CONNECTING) return;

    if (WiFi.status() == WL_CONNECTED) {
        state = State::CONNECTED;
        has_connected_once = true;
        initial_scan_complete = true;
        const String ip = WiFi.localIP().toString();
        pro_log_info("NETWORK", "connected ssid=\"%s\" ip=%s",
                     profiles[active_profile].ssid, ip.c_str());
        updateSignalLevel(now_ms);
        return;
    }

    if (static_cast<uint32_t>(now_ms - attempt_started_ms) < connect_timeout_ms) return;

    if (attempts_started <= retries_per_profile) {
        pro_log_warn("NETWORK", "connection timeout ssid=\"%s\"; retrying",
                     profiles[active_profile].ssid);
        startAttempt(now_ms);
        return;
    }

    pro_log_error("NETWORK", "connection failed ssid=\"%s\" attempts=%u",
                  profiles[active_profile].ssid,
                  static_cast<unsigned>(attempts_started));
    ++active_profile;
    attempts_started = 0;
    if (active_profile < profile_count) startAttempt(now_ms);
    else exhaustProfiles();
}

bool isConnected() {
    return state == State::CONNECTED && WiFi.status() == WL_CONNECTED;
}

NetworkState getState() {
    switch (state) {
        case State::CONNECTING:
            return has_connected_once ? NetworkState::RECONNECTING : NetworkState::CONNECTING;
        case State::CONNECTED:
            return NetworkState::CONNECTED;
        case State::EXHAUSTED:
            return NetworkState::ERROR;
        case State::IDLE:
        default:
            return NetworkState::UNINITIALIZED;
    }
}

uint8_t signalLevel() {
    return isConnected() ? signal_level : 0;
}

bool initialScanComplete() {
    return initial_scan_complete;
}
} // namespace NetworkService
} // namespace PDACYD
