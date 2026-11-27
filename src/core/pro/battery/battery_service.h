#pragma once

#include <stdint.h>

namespace PDACYD {
namespace BatteryService {

void begin();
uint8_t readPercent();

} // namespace BatteryService
} // namespace PDACYD
