#include "pro_logger.h"

#include "../storage/sd_storage.h"
#include "../storage/storage_config.h"
#include <Arduino.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

namespace PDACYD {
namespace {
bool sd_logging_enabled = true;

void writeLog(const char *level, const char *tag, const char *format, va_list args) {
    char message[180];
    vsnprintf(message, sizeof(message), format, args);

    char timestamp[24];
    const time_t now = time(nullptr);
    if (now >= 1704067200) {
        struct tm local_time{};
        if (localtime_r(&now, &local_time) != nullptr) {
            snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02d %02d:%02d:%02d",
                     local_time.tm_year + 1900, local_time.tm_mon + 1,
                     local_time.tm_mday, local_time.tm_hour, local_time.tm_min,
                     local_time.tm_sec);
        } else {
            snprintf(timestamp, sizeof(timestamp), "UP+%lu ms",
                     static_cast<unsigned long>(millis()));
        }
    } else {
        snprintf(timestamp, sizeof(timestamp), "UP+%lu ms",
                 static_cast<unsigned long>(millis()));
    }

    char line[256];
    snprintf(line, sizeof(line), "%s [%s] [%s] %s", timestamp,
             level, tag != nullptr ? tag : "PRO", message);
    Serial.println(line);
    if (sd_logging_enabled) {
        (void)SDStorage::appendLog(SDStorage::SYSTEM_LOG, line);
    }
}
} // namespace

void pro_logger_set_sd_enabled(bool enabled) {
    sd_logging_enabled = enabled;
}

void pro_log_info(const char *tag, const char *format, ...) {
    va_list args;
    va_start(args, format);
    writeLog("INFO", tag, format, args);
    va_end(args);
}

void pro_log_warn(const char *tag, const char *format, ...) {
    va_list args;
    va_start(args, format);
    writeLog("WARN", tag, format, args);
    va_end(args);
}

void pro_log_error(const char *tag, const char *format, ...) {
    va_list args;
    va_start(args, format);
    writeLog("ERROR", tag, format, args);
    va_end(args);
}
} // namespace PDACYD
