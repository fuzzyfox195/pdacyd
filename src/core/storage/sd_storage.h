#pragma once
#include <Arduino.h>

namespace SDStorage {
bool begin();
bool isMounted();
bool readJson(const char *path, String &out);
bool writeJsonAtomic(const char *path, const String &json);
bool appendLog(const char *path, const char *line);
}
