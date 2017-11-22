#include "hysteresis.h"

/**
 * @brief Constructor: Set default values
 *
 * @param settings
 */
HysteresisClass::HysteresisClass(SettingsClass* settings) : SETTINGS(settings) {
    heatCycleIsActive = false;
}

/**
 * @brief Run autotune or get the values from the PID
 *
 * @param input   the measured temperature
 * @param setPoint   the desired temperature
 * @return float  the output value, ranging between 0 and 1
 */
float HysteresisClass::Loop(const float input, const float setPoint) {
    float output = 0;
    float low = setPoint - SETTINGS->TheSettings.HysteresisRange;
    float high = setPoint /*+ SETTINGS->TheSettings.HysteresisRange*/;
    if (input < low || (heatCycleIsActive && input < high)) {
        output = min(
            ((setPoint - input) / 3.0),
            1.0);
    }
    if (input >= high)
        heatCycleIsActive = false;
    return output;
}
