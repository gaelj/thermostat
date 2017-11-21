#include "hysteresis.h"

/**
 * @brief Constructor: Set default values
 *
 * @param settings
 */
HysteresisClass::HysteresisClass(SettingsClass* settings) : SETTINGS(settings) {
    Serial.println("*** Init Hyst");
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
    Serial.println("hyst loop");
    float output = 0;
    float low = setPoint - SETTINGS->TheSettings.HysteresisRange;
    float high = setPoint /*+ SETTINGS->TheSettings.HysteresisRange*/;
    if (input < low || (heatCycleIsActive && input < high)) {
        Serial.print("hyst updt out ");
        Serial.print(setPoint);
        Serial.print(" ");
        Serial.print(SETTINGS->TheSettings.HysteresisRange);
        Serial.print(" ");
        Serial.print(input);
        Serial.print(" -> ");

        output = min(
            ((setPoint - input) / 5.0),
            1.0);
        Serial.print(output);
    }
    if (input >= high)
        heatCycleIsActive = false;
    return output;
}
