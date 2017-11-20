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

    float temp = 0;
    float output = 0;
    byte state = 0;
    temp = SENSOR->GetTemperature();
    
    if (windowStartTime == 0 || ((millis() - windowStartTime) > SETTINGS->TheSettings.SampleTime)) {
        //time to shift the Relay Window
        output = HYST->Loop(temp);
        windowStartTime = millis();
    }
    // output = AUTOPID->Loop(temp);
    state = GetBoilerStateByWindowWidth(output);
    BOILER->SetBoilerState(state);
    
    return 0;
}

/**
 * @brief Get the current boiler state based on autopid output
 * 
 * @return byte 
 */
byte ThermostatClass::GetBoilerStateByWindowWidth(float output) {
    unsigned long now = millis();
    bool state = ((now - windowStartTime) < output);
    
    Serial.print("Get boiler state");
    Serial.print(" window: ");
    Serial.print((now - windowStartTime) * 100 / SETTINGS->TheSettings.SampleTime);
    Serial.print("% output: ");
    Serial.print(output * 100 / SETTINGS->TheSettings.SampleTime);
    Serial.print("% state:");
    Serial.println(state);
    
    return state;
}
