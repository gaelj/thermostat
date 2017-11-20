#include "hysteresis.h"

/**
 * @brief Constructor: Set default values
 * 
 * @param pid 
 * @param atune 
 * @param settings 
 */
HysteresisClass::HysteresisClass(SettingsClass* settings): SETTINGS(settings) {
    Input = 0;
    Output = 0;
    Serial.println("*** Init Hyst");
}

/**
 * @brief Print an information string on Serial
 * 
 */
void HysteresisClass::SerialPrintInfoString() {
    Serial.print("s: "); Serial.print(SETTINGS->TheSettings.Setpoint); Serial.print(" ");
    Serial.print("i: "); Serial.print(Input); Serial.print(" ");
    Serial.print("o: "); Serial.println(Output);
}

/**
 * @brief Run autotune or get the values from the PID
 * 
 * @param input   the PID input value
 * @return float  the PID output value
 */
unsigned long HysteresisClass::Loop(float input) {
    Serial.println("hyst loop");
    Input = input;
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
    if (Input < SETTINGS->TheSettings.Setpoint) { // + SETTINGS->TheSettings.HysteresisRange) {
        Serial.print("hyst updt out ");
        Serial.print(SETTINGS->TheSettings.Setpoint);
        Serial.print(" ");
        Serial.print(SETTINGS->TheSettings.HysteresisRange);
        Serial.print(" ");
        Serial.print(Input);
        Serial.print(" ");

        float output = 1 / min((SETTINGS->TheSettings.Setpoint + SETTINGS->TheSettings.HysteresisRange - Input) / 5 + 0.15, 1.0);
        Serial.print(output);
        Serial.print(" ");
        Serial.print(SETTINGS->TheSettings.SampleTime);
        Serial.print(" ");
        Output = SETTINGS->TheSettings.SampleTime / output;
        Serial.println(Output);
    }
    
    SerialPrintInfoString();
    return Output;
}
