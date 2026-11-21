#include "time_service.h"

#include "../../storage/sd_storage.h"
#include "../../storage/storage_config.h"
#include "../pro_logger.h"
#include "time_now_root_ca.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <esp_sntp.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

namespace PDACYD {
namespace {
constexpr char POSIX_TIMEZONE[] = "ICT-7";
constexpr char TIME_API_URL[] =
    "https://time.now/developer/api/time?timezone=Asia%2FHo_Chi_Minh";
constexpr uint32_t CLOCK_SAVE_MS = 60UL * 1000UL;
constexpr uint32_t API_RETRY_MS = 5UL * 60UL * 1000UL;
constexpr uint32_t API_RESYNC_MS = 60UL * 60UL * 1000UL;
constexpr uint32_t API_TIMEOUT_MS = 10UL * 1000UL;

uint32_t last_persist_ms;
uint32_t last_sync_attempt_ms;
bool sync_requested;
bool started;
bool sd_ready;
bool clock_valid;
bool synchronized;
bool api_sync_failed;
bool fallback_clock;
bool tls_time_bootstrap_required;
bool previous_network_connected;
char timezone_name[40] = "Asia/Ho_Chi_Minh";
ClockSnapshot current_snapshot{};

void updateSnapshot();

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

bool readBoolean(const String &json, const char *key, bool &value) {
    const String needle = String("\"") + key + "\"";
    int position = json.indexOf(needle);
    if (position < 0) return false;
    position = json.indexOf(':', position + needle.length());
    if (position < 0) return false;
    ++position;
    while (position < json.length() && (json[position] == ' ' || json[position] == '\t' ||
           json[position] == '\r' || json[position] == '\n')) ++position;
    if (json.startsWith("true", position)) {
        value = true;
        return true;
    }
    if (json.startsWith("false", position)) {
        value = false;
        return true;
    }
    return false;
}

void setFallbackClock() {
    struct tm fallback{};
    fallback.tm_year = 2024 - 1900;
    fallback.tm_mon = 0;
    fallback.tm_mday = 1;
    fallback.tm_hour = 0;
    fallback.tm_isdst = -1;
    const time_t local_epoch = mktime(&fallback);
    if (local_epoch <= 0) return;

    struct timeval tv{};
    tv.tv_sec = local_epoch;
    tv.tv_usec = 0;
    if (settimeofday(&tv, nullptr) == 0) {
        clock_valid = true;
        synchronized = false;
        fallback_clock = true;
        tls_time_bootstrap_required = true;
        updateSnapshot();
        pro_log_warn("TIME", "using fallback date 2024-01-01 00:00");
    }
}

bool readApiMicroseconds(const String &json, uint32_t &microseconds) {
    const int datetime_key = json.indexOf("\"datetime\"");
    if (datetime_key < 0) return false;
    const int decimal = json.indexOf('.', datetime_key);
    if (decimal < 0) return false;

    uint32_t fraction = 0;
    int digits = 0;
    for (int i = decimal + 1; i < json.length() && digits < 6; ++i) {
        const char ch = json[i];
        if (ch < '0' || ch > '9') break;
        fraction = fraction * 10 + static_cast<uint32_t>(ch - '0');
        ++digits;
    }
    if (digits == 0) return false;
    while (digits++ < 6) fraction *= 10;
    microseconds = fraction;
    return true;
}

void restoreCache() {
    if (!sd_ready || !SDStorage::ensureDirectory("/data")) return;

    const String initial = String("{\n") +
            "  \"schema_version\": 1,\n" +
            "  \"timezone\": \"Asia/Ho_Chi_Minh\",\n" +
            "  \"last_sync_unix\": 0,\n" +
            "  \"last_sync_ok\": false,\n" +
            "  \"last_sync_hour\": 0,\n" +
            "  \"last_sync_min\": 0,\n" +
            "  \"last_sync_sec\": 0,\n" +
            "  \"last_sync_day\": 0,\n" +
            "  \"last_sync_month\": 0,\n" +
            "  \"last_sync_year\": 0\n" +
            "}\n";
    if (!SDStorage::ensureJsonFile(SDStorage::CLOCK_DATA, initial)) {
        pro_log_error("TIME", "could not create /data/clock.json");
        return;
    }

    String json;
    if (!SDStorage::readJson(SDStorage::CLOCK_DATA, json)) {
        pro_log_error("TIME", "could not read /data/clock.json");
        return;
    }

    int64_t unix_time = 0;
    if (!readNumber(json, "last_sync_unix", unix_time) || unix_time <= 0) return;
    bool previously_synced = false;
    const bool has_sync_status = readBoolean(json, "last_sync_ok", previously_synced);

    struct timeval tv{};
    tv.tv_sec = static_cast<time_t>(unix_time);
    tv.tv_usec = 0;
    if (settimeofday(&tv, nullptr) == 0) {
        clock_valid = true;
        tls_time_bootstrap_required = !has_sync_status || !previously_synced;
        pro_log_info("TIME", "restored clock cache unix=%lld",
                     static_cast<long long>(unix_time));
    }
}

void updateSnapshot() {
    const time_t now = time(nullptr);
    current_snapshot = {};
    current_snapshot.unix_time = now;
    current_snapshot.synchronized = synchronized;
    current_snapshot.last_sync_ms = last_sync_attempt_ms;
    current_snapshot.valid = clock_valid && now > 0;
    if (!current_snapshot.valid) return;

    struct tm local_time{};
    if (!localtime_r(&now, &local_time)) {
        current_snapshot.valid = false;
        return;
    }

    current_snapshot.year = static_cast<uint16_t>(local_time.tm_year + 1900);
    current_snapshot.month = static_cast<uint8_t>(local_time.tm_mon + 1);
    current_snapshot.day = static_cast<uint8_t>(local_time.tm_mday);
    current_snapshot.hour = static_cast<uint8_t>(local_time.tm_hour);
    current_snapshot.minute = static_cast<uint8_t>(local_time.tm_min);
    current_snapshot.second = static_cast<uint8_t>(local_time.tm_sec);
    current_snapshot.weekday = static_cast<uint8_t>(local_time.tm_wday);
}

bool bootstrapClockFromNtp() {
    const char *posix_timezone = strcmp(timezone_name, "Asia/Ho_Chi_Minh") == 0 ?
                                 POSIX_TIMEZONE : timezone_name;
    esp_sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
    pro_log_warn("TIME", "NTP bootstrap started for HTTPS certificate validation");
    configTzTime(posix_timezone, "pool.ntp.org", "time.nist.gov");
    struct tm time_info{};
    if (!getLocalTime(&time_info, API_TIMEOUT_MS)) {
        pro_log_error("TIME", "NTP bootstrap failed; HTTPS certificate time is unknown");
        return false;
    }

    // After TLS has a usable clock, use time.now as the sole sync source.
    esp_sntp_stop();
    clock_valid = true;
    fallback_clock = false;
    tls_time_bootstrap_required = false;
    pro_log_info("TIME", "NTP bootstrap completed; SNTP stopped");
    return true;
}

bool fetchTimeApi(struct timeval &api_time) {
    pro_log_info("TIME", "preparing HTTPS time API request");
    WiFiClientSecure tls_client;
    tls_client.setCACert(TIME_NOW_ROOT_CA);

    HTTPClient http;
    http.setTimeout(API_TIMEOUT_MS);
    if (!http.begin(tls_client, TIME_API_URL)) {
        pro_log_error("TIME", "could not open HTTPS time API connection");
        return false;
    }
    pro_log_info("TIME", "HTTPS client opened; starting request");
    http.addHeader("Accept", "application/json");

    const uint32_t request_started_ms = millis();
    const int status = http.GET();
    const uint32_t request_duration_ms = millis() - request_started_ms;
    if (status != HTTP_CODE_OK) {
        pro_log_error("TIME", "HTTP GET /time failed code=%d", status);
        http.end();
        return false;
    }

    const String response = http.getString();
    http.end();
    pro_log_info("TIME", "clock JSON received http=%d bytes=%u", status,
                 static_cast<unsigned>(response.length()));

    int64_t unix_time = 0;
    uint32_t fractional_us = 0;
    if (!readNumber(response, "unixtime", unix_time) || unix_time < 1700000000LL) {
        pro_log_error("TIME", "clock JSON parse failed: missing/invalid unixtime");
        return false;
    }
    (void)readApiMicroseconds(response, fractional_us);

    api_time.tv_sec = static_cast<time_t>(unix_time);
    // The endpoint timestamp is generated during the request. Add half the
    // round trip so local second changes track the server instead of request start.
    api_time.tv_usec = static_cast<suseconds_t>(fractional_us + request_duration_ms * 500UL);
    if (api_time.tv_usec >= 1000000) {
        api_time.tv_sec += api_time.tv_usec / 1000000;
        api_time.tv_usec %= 1000000;
    }
    return true;
}

bool syncTimeNow(uint32_t now_ms) {
    last_sync_attempt_ms = now_ms;
    pro_log_info("TIME", "requesting current time from time.now");
    // A first boot has no valid clock to check HTTPS certificate dates. Use
    // NTP only to bootstrap TLS, then immediately replace it with time.now.
    if (tls_time_bootstrap_required && !bootstrapClockFromNtp()) return false;

    struct timeval api_time{};
    if (!fetchTimeApi(api_time)) return false;
    if (settimeofday(&api_time, nullptr) != 0) {
        pro_log_error("TIME", "could not apply HTTP clock timestamp");
        return false;
    }

    synchronized = true;
    clock_valid = true;
    fallback_clock = false;
    tls_time_bootstrap_required = false;
    updateSnapshot();
    pro_log_info("TIME", "time.now synchronized unix=%lld.%06ld",
                 static_cast<long long>(api_time.tv_sec),
                 static_cast<long>(api_time.tv_usec));
    return true;
}

void persistClock() {
    if (!sd_ready || !clock_valid || !synchronized || fallback_clock) return;

    const time_t now = time(nullptr);
    struct tm local_time{};
    if (now <= 0 || !localtime_r(&now, &local_time)) return;

    char json[384];
    const int length = snprintf(json, sizeof(json),
        "{\n"
        "  \"schema_version\": 1,\n"
        "  \"timezone\": \"%s\",\n"
        "  \"last_sync_unix\": %lld,\n"
        "  \"last_sync_ok\": %s,\n"
        "  \"last_sync_hour\": %d,\n"
        "  \"last_sync_min\": %d,\n"
        "  \"last_sync_sec\": %d,\n"
        "  \"last_sync_day\": %d,\n"
        "  \"last_sync_month\": %d,\n"
        "  \"last_sync_year\": %d\n"
        "}\n",
        timezone_name, static_cast<long long>(now), synchronized ? "true" : "false",
        local_time.tm_hour, local_time.tm_min, local_time.tm_sec,
        local_time.tm_mday, local_time.tm_mon + 1, local_time.tm_year + 1900);
    if (length > 0 && static_cast<size_t>(length) < sizeof(json)) {
        if (!SDStorage::writeJsonAtomic(SDStorage::CLOCK_DATA, String(json))) {
            pro_log_error("TIME", "could not save /data/clock.json");
        }
    }
}
} // namespace

bool TimeService::begin() {
    if (started) return true;
    pro_log_info("TIME", "time service started on PRO core");

    setenv("TZ", POSIX_TIMEZONE, 1);
    tzset();

    sd_ready = SDStorage::isMounted();
    if (!sd_ready) sd_ready = SDStorage::begin();
    if (!sd_ready) {
        pro_log_error("TIME", "SD unavailable; clock cache disabled");
    } else {
        restoreCache();
    }

    if (!clock_valid) setFallbackClock();

    started = true;
    last_persist_ms = millis();
    updateSnapshot();
    return true;
}

ClockSnapshot TimeService::getSnapshot() const {
    return current_snapshot;
}

bool TimeService::isSynchronized() const {
    return synchronized;
}

bool TimeService::requestSync() {
    sync_requested = true;
    return true;
}

void TimeService::setTimezone(const char *timezone) {
    if (timezone == nullptr || timezone[0] == '\0') return;
    strncpy(timezone_name, timezone, sizeof(timezone_name) - 1);
    timezone_name[sizeof(timezone_name) - 1] = '\0';
    const char *posix_timezone = strcmp(timezone, "Asia/Ho_Chi_Minh") == 0 ?
                                 POSIX_TIMEZONE : timezone;
    setenv("TZ", posix_timezone, 1);
    tzset();
}

void TimeService::poll(uint32_t now_ms, bool network_connected,
                       bool initial_scan_complete) {
    if (!started) return;

    if (network_connected) {
        if (!previous_network_connected) sync_requested = true;
        previous_network_connected = true;
        fallback_clock = false;
    } else {
        previous_network_connected = false;
        if (initial_scan_complete && !fallback_clock) setFallbackClock();
    }

    const bool retry_due = !last_sync_attempt_ms ||
        static_cast<uint32_t>(now_ms - last_sync_attempt_ms) >=
            ((api_sync_failed || !synchronized) ? API_RETRY_MS : API_RESYNC_MS);
    const bool should_sync = sync_requested || (network_connected && retry_due);
    sync_requested = false;
    if (should_sync && network_connected) {
        api_sync_failed = !syncTimeNow(now_ms);
    }

    // Sample wall time on every PRO tick and publish at each Unix second
    // boundary. Keep the requested 2024 fallback fixed while no Wi-Fi profile
    // can connect; a real cache/API/NTP clock continues normally.
    if (!fallback_clock) {
        const time_t wall_second = time(nullptr);
        if (wall_second != current_snapshot.unix_time ||
            current_snapshot.valid != (clock_valid && wall_second > 0)) {
            updateSnapshot();
        }
    }
    if (static_cast<uint32_t>(now_ms - last_persist_ms) >= CLOCK_SAVE_MS) {
        last_persist_ms = now_ms;
        persistClock();
    }
}
} // namespace PDACYD
