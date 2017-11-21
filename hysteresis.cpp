#include "hysteresis.h"

/**
 * @brief Constructor: Set default values
 * 
 * @param settings 
 */
HysteresisClass::HysteresisClass(SettingsClass* settings): SETTINGS(settings) {
    Serial.println("*** Init Hyst");
    heatCycleIsActive = false;
}

/**
 * @brief Run autotune or get the values from the PID
 * 
 * @param input   the PID input value
 * @return float  the PID output value
 */
float HysteresisClass::Loop(const float input) {
    Serial.println("hyst loop");
    float output = 0;
    float low = SETTINGS->TheSettings.Setpoint - SETTINGS->TheSettings.HysteresisRange;
    float high = SETTINGS->TheSettings.Setpoint /*+ SETTINGS->TheSettings.HysteresisRange*/;
    if (input < low || (heatCycleIsActive && input < high)) {
        Serial.print("hyst updt out ");
        Serial.print(SETTINGS->TheSettings.Setpoint);
        Serial.print(" ");
        Serial.print(SETTINGS->TheSettings.HysteresisRange);
        Serial.print(" ");
        Serial.print(input);
        Serial.print(" -> ");

        output = min(
            ((SETTINGS->TheSettings.Setpoint - input) / 5.0),
            1.0);
        Serial.print(output);
    }
    if (input >= high)
        heatCycleIsActive = false;
    return output;
}
