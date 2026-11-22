#pragma once

#include <stdint.h>
#include "../../system/system_types.h"

namespace PDACYD {
namespace NetworkService {
bool begin();
void poll(uint32_t now_ms);
bool isConnected();
NetworkState getState();
uint8_t signalLevel();
bool initialScanComplete();
}
} // namespace PDACYD
