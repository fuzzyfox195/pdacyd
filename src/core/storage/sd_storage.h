#pragma once
#include <Arduino.h>

namespace SDStorage {
bool begin();
bool isMounted();
bool ensureDirectory(const char *path);
bool resetLog(const char *path);
bool ensureJsonFile(const char *path, const String &initial_json);
bool readJson(const char *path, String &out);
bool readTextTail(const char *path, String &out, size_t max_bytes);
bool writeJsonAtomic(const char *path, const String &json);
bool appendLog(const char *path, const char *line);
}
