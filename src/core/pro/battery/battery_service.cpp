#include "battery_service.h"

#include <Arduino.h>
#include <esp32-hal-adc.h>

namespace PDACYD {
namespace BatteryService {
namespace {

constexpr uint8_t BATTERY_ADC_PIN = 9;
constexpr uint8_t ADC_SAMPLE_COUNT = 8;

struct VoltagePoint {
    uint16_t millivolts;
    uint8_t percent;
};

// Typical single-cell Li-ion open-circuit voltage curve. GPIO9 is behind
// the board's 1:1 resistor divider, so the measured ADC voltage is doubled.
constexpr VoltagePoint BATTERY_CURVE[] = {
    {3300, 0}, {3500, 5}, {3600, 10}, {3700, 20}, {3750, 30},
    {3800, 40}, {3850, 50}, {3900, 60}, {3950, 70}, {4000, 80},
    {4100, 90}, {4200, 100}
};

uint8_t voltage_to_percent(uint16_t battery_mv)
{
    constexpr size_t point_count = sizeof(BATTERY_CURVE) / sizeof(BATTERY_CURVE[0]);
    if (battery_mv <= BATTERY_CURVE[0].millivolts) return 0;
    if (battery_mv >= BATTERY_CURVE[point_count - 1].millivolts) return 100;

    for (size_t i = 1; i < point_count; ++i) {
        const VoltagePoint &lower = BATTERY_CURVE[i - 1];
        const VoltagePoint &upper = BATTERY_CURVE[i];
        if (battery_mv <= upper.millivolts) {
            const uint16_t voltage_span = upper.millivolts - lower.millivolts;
            const uint8_t percent_span = upper.percent - lower.percent;
            return lower.percent +
                (uint32_t)(battery_mv - lower.millivolts) * percent_span / voltage_span;
        }
    }
    return 100;
}

} // namespace

void begin()
{
    analogReadResolution(12);
    analogSetPinAttenuation(BATTERY_ADC_PIN, ADC_11db);
}

uint8_t readPercent()
{
    uint32_t adc_mv_sum = 0;
    for (uint8_t sample = 0; sample < ADC_SAMPLE_COUNT; ++sample) {
        adc_mv_sum += analogReadMilliVolts(BATTERY_ADC_PIN);
    }

    const uint16_t adc_mv = adc_mv_sum / ADC_SAMPLE_COUNT;
    const uint16_t battery_mv = adc_mv * 2;
    return voltage_to_percent(battery_mv);
}

} // namespace BatteryService
} // namespace PDACYD
