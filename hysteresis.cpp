#include "hysteresis.h"

/**
 * @brief Constructor: Set default values
 * 
 * @param pid 
 * @param atune 
 * @param settings 
 */
HysteresisClass::HysteresisClass(SettingsClass* settings): SETTINGS(settings) {
    Serial.println("*** Init Hyst");
}

/**
 * @brief Run autotune or get the values from the PID
 * 
 * @param input   the PID input value
 * @return float  the PID output value
 */
float HysteresisClass::Loop(const float input) {
    Serial.println("hyst loop");
    /*
    Serial.print(" SP ");
    Serial.print(SETTINGS->TheSettings.Setpoint);
    Serial.print(" HR ");
    Serial.print((float)SETTINGS->TheSettings.HysteresisRange);
    Serial.print(" SP + HR ");
    Serial.print(SETTINGS->TheSettings.Setpoint + SETTINGS->TheSettings.HysteresisRange);
    Serial.print(" I ");
    Serial.println(Input);
    */
    float output = 0;
    if (input < SETTINGS->TheSettings.Setpoint) { // + SETTINGS->TheSettings.HysteresisRange) {
        Serial.print("hyst updt out ");
        Serial.print(SETTINGS->TheSettings.Setpoint);
        Serial.print(" ");
        Serial.print(SETTINGS->TheSettings.HysteresisRange);
        Serial.print(" ");
        Serial.print(input);
        Serial.print(" -> ");

        output = min(
            ((SETTINGS->TheSettings.Setpoint + SETTINGS->TheSettings.HysteresisRange - input)
                / 5.0) + 0.15,
            1.0);
        Serial.print(output);
    }
    return output;
}
