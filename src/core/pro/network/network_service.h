#pragma once

#include <stdint.h>

namespace PDACYD {
namespace NetworkService {
bool begin();
void poll(uint32_t now_ms);
bool isConnected();
bool initialScanComplete();
}
} // namespace PDACYD
