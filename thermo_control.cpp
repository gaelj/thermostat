/**
 * @brief Main Thermostat class
 *          Gives access to desired temperature, real temperature & humidity
 * 
 */

#include "thermo_control.h"

/**
 * @brief Constructor. Turns off the boiler
 * 
 */
ThermostatClass::ThermostatClass(AutoPidClass* autoPid, SettingsClass* settings, SensorClass* sensor, BoilerClass* boiler, HysteresisClass* hist):
        AUTOPID(autoPid), SETTINGS(settings), SENSOR(sensor), BOILER(boiler), HYST(hist) {
    BOILER->SetBoilerState(false);
    windowStartTime = 0;
    LastOutput = 0;
}

/**
 * @brief Set the desired room temperature
 * 
 * @param value     the desired temperature
 */
void ThermostatClass::SetSetpoint(float value) {
    if (value < THERMOSTAT_MIN || value > THERMOSTAT_MAX) value = THERMOSTAT_DEFAULT;
    if (SETTINGS->TheSettings.Setpoint != value) {
        SETTINGS->TheSettings.Setpoint = value;
        SETTINGS->PersistSettings();
    }
    windowStartTime = 0;
}

/**
 * @brief Get the desired room temperature
 * 
 * @return float   the desired temperature
 */
float ThermostatClass::GetSetpoint() {
    return SETTINGS->TheSettings.Setpoint;
}

/**
 * @brief MCU loop function
 * 
 * @return int      0 if OK, -1 in case of error
 */
int ThermostatClass::Loop() {
    SENSOR->ReadSensor();

    float temp = SENSOR->GetTemperature();
    
    if (windowStartTime == 0 || ((millis() - windowStartTime) > SETTINGS->TheSettings.SampleTime)) {
        //time to shift the Relay Window
        Serial.println("New window **");
        LastOutput = HYST->Loop(temp);
        windowStartTime = millis();
    }
    // output = AUTOPID->Loop(temp);
    byte state = GetBoilerStateByWindowWidth(LastOutput);
    BOILER->SetBoilerState(state);
    
    return 0;
}

/**
 * @brief Get the current boiler state based on autopid output
 * 
 * @return byte 
 */
byte ThermostatClass::GetBoilerStateByWindowWidth(const float output) {
    const unsigned long now = millis();
    const bool state = ((now - windowStartTime) < (output * SETTINGS->TheSettings.SampleTime));
    
    Serial.print("Get boiler state");
    Serial.print(" window: ");
    Serial.print((now - windowStartTime) * 100 / SETTINGS->TheSettings.SampleTime);
    Serial.print("% output: ");
    Serial.print(output * 100);
    Serial.print("% state:");
    Serial.println(state);
    
    return state;
}
