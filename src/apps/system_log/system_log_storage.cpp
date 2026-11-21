#include "system_log_storage.h"

#include "../../core/storage/sd_storage.h"
#include "../../core/storage/storage_config.h"

extern "C" bool app_system_log_read_tail(char *buffer, size_t capacity) {
    if (buffer == nullptr || capacity < 2) return false;
    String contents;
    if (!SDStorage::readTextTail(SDStorage::SYSTEM_LOG, contents, capacity - 1)) {
        return false;
    }
    contents.toCharArray(buffer, capacity);
    return true;
}
