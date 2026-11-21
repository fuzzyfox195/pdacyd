#include "sd_storage.h"
#include <SD_MMC.h>
#include <FS.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace {
static constexpr size_t LOG_ROTATE_LIMIT = 96 * 1024;
static constexpr int SD_CLK = 38;
static constexpr int SD_CMD = 40;
static constexpr int SD_D0  = 39;
static constexpr int SD_D1  = 41;
static constexpr int SD_D2  = 48;
static constexpr int SD_D3  = 47;
static constexpr char *MOUNT_POINT = const_cast<char *>("/sdcard");
static bool mounted = false;
static SemaphoreHandle_t storageMutex = nullptr;

bool ensureParent(const char *path) {
  String p(path);
  const int last = p.lastIndexOf('/');
  if (last <= 0) return true;
  String dir = p.substring(0, last);
  if (SD_MMC.exists(dir)) return true;
  return SD_MMC.mkdir(dir);
}
}

namespace SDStorage {
bool begin() {
  if (mounted) return true;
  if (!storageMutex) storageMutex = xSemaphoreCreateMutex();
  if (!storageMutex) return false;
  SD_MMC.setPins(SD_CLK, SD_CMD, SD_D0, SD_D1, SD_D2, SD_D3);
  mounted = SD_MMC.begin(MOUNT_POINT, false, false, SDMMC_FREQ_DEFAULT, 5);
  if (!mounted) {
    Serial.println("[SD] mount failed");
    return false;
  }
  if (SD_MMC.cardType() == CARD_NONE) {
    Serial.println("[SD] card detection failed");
    SD_MMC.end();
    mounted = false;
    return false;
  }
  Serial.printf("[SD] mounted at %s, card size: %llu MB\n", MOUNT_POINT,
                SD_MMC.cardSize() / (1024ULL * 1024ULL));
  SD_MMC.mkdir("/config");
  SD_MMC.mkdir("/data");
  SD_MMC.mkdir("/logs");
  SD_MMC.mkdir("/meta");
  return true;
}

bool isMounted() { return mounted; }

bool ensureDirectory(const char *path) {
  if (!mounted || !path || !storageMutex) return false;
  if (xSemaphoreTake(storageMutex, pdMS_TO_TICKS(2000)) != pdTRUE) return false;
  const bool ok = SD_MMC.exists(path) || SD_MMC.mkdir(path) || SD_MMC.exists(path);
  xSemaphoreGive(storageMutex);
  return ok;
}

bool resetLog(const char *path) {
  if (!mounted || !path || !storageMutex) return false;
  if (xSemaphoreTake(storageMutex, pdMS_TO_TICKS(2000)) != pdTRUE) return false;

  bool ok = ensureParent(path);
  const String rotated = String(path) + ".1";
  if (ok && SD_MMC.exists(rotated.c_str())) {
    ok = SD_MMC.remove(rotated.c_str());
  }
  if (ok && SD_MMC.exists(path)) ok = SD_MMC.remove(path);
  if (ok) {
    File f = SD_MMC.open(path, FILE_WRITE);
    if (!f) ok = false;
    else {
      f.flush();
      ok = f.size() == 0;
      f.close();
    }
  }

  xSemaphoreGive(storageMutex);
  return ok;
}

bool ensureJsonFile(const char *path, const String &initial_json) {
  if (!mounted || !path || !storageMutex) return false;
  if (xSemaphoreTake(storageMutex, pdMS_TO_TICKS(2000)) != pdTRUE) return false;
  bool ok = true;
  if (!SD_MMC.exists(path)) {
    const String tmp = String(path) + ".tmp";
    ok = ensureParent(path);
    if (ok) {
      File f = SD_MMC.open(tmp, FILE_WRITE);
      if (!f) ok = false;
      else {
        const size_t written = f.print(initial_json);
        f.flush();
        f.close();
        ok = written == initial_json.length();
      }
    }
    if (ok) ok = SD_MMC.rename(tmp, path);
    if (!ok && SD_MMC.exists(tmp)) SD_MMC.remove(tmp);
  }
  xSemaphoreGive(storageMutex);
  return ok;
}

bool readJson(const char *path, String &out) {
  if (!mounted || !path || !storageMutex) return false;
  if (xSemaphoreTake(storageMutex, pdMS_TO_TICKS(2000)) != pdTRUE) return false;
  File f = SD_MMC.open(path, FILE_READ);
  bool ok = false;
  if (f) {
    out = f.readString();
    ok = out.length() > 0;
    f.close();
  }
  xSemaphoreGive(storageMutex);
  return ok;
}

bool readTextTail(const char *path, String &out, size_t max_bytes) {
  if (!mounted || !path || max_bytes == 0 || !storageMutex) return false;
  if (xSemaphoreTake(storageMutex, pdMS_TO_TICKS(3000)) != pdTRUE) return false;
  File f = SD_MMC.open(path, FILE_READ);
  bool ok = false;
  if (f) {
    const size_t file_size = f.size();
    const size_t start = file_size > max_bytes ? file_size - max_bytes : 0;
    ok = start == 0 || f.seek(start);
    if (ok) {
      out = f.readString();
      if (start > 0) {
        const int newline = out.indexOf('\n');
        if (newline >= 0) out.remove(0, newline + 1);
        else out = "";
      }
    }
    f.close();
  }
  xSemaphoreGive(storageMutex);
  return ok;
}

bool writeJsonAtomic(const char *path, const String &json) {
  if (!mounted || !path || !storageMutex) return false;
  if (xSemaphoreTake(storageMutex, pdMS_TO_TICKS(2000)) != pdTRUE) return false;
  const String tmp = String(path) + ".tmp";
  bool ok = ensureParent(path);
  if (ok) {
    File f = SD_MMC.open(tmp, FILE_WRITE);
    if (!f) ok = false;
    else {
      const size_t written = f.print(json);
      f.flush();
      f.close();
      ok = written == json.length();
    }
  }
  if (ok) {
    if (SD_MMC.exists(path)) SD_MMC.remove(path);
    ok = SD_MMC.rename(tmp, path);
  }
  if (!ok && SD_MMC.exists(tmp)) SD_MMC.remove(tmp);
  xSemaphoreGive(storageMutex);
  return ok;
}

bool appendLog(const char *path, const char *line) {
  if (!mounted || !path || !line || !storageMutex) return false;
  if (xSemaphoreTake(storageMutex, pdMS_TO_TICKS(2000)) != pdTRUE) return false;
  bool ok = ensureParent(path);
  if (ok && SD_MMC.exists(path)) {
    File current = SD_MMC.open(path, FILE_READ);
    const size_t current_size = current ? current.size() : 0;
    if (current) current.close();
    if (current_size >= LOG_ROTATE_LIMIT) {
      const String rotated = String(path) + ".1";
      if (SD_MMC.exists(rotated)) SD_MMC.remove(rotated);
      if (!SD_MMC.rename(path, rotated)) ok = false;
    }
  }
  if (ok) {
    File f = SD_MMC.open(path, FILE_APPEND);
    if (!f) ok = false;
    else {
      ok = f.println(line) > 0;
      f.flush();
      f.close();
    }
  }
  xSemaphoreGive(storageMutex);
  return ok;
}
}
