/**
 * @brief Main Thermostat class
 *          Gives access to desired temperature, real temperature & humidity
 * 
 */

#include "thermo_control.h"

/**
 * @brief Constructor. Does required initialisations
 * 
 */
ThermostatClass::ThermostatClass(AutoPidClass* autoPid, SettingsClass* settings, SensorClass* sensor, BoilerClass* boiler):
        AUTOPID(autoPid), SETTINGS(settings), SENSOR(sensor), BOILER(boiler) { }

/**
 * @brief Apply the settings
 * 
 */
void ThermostatClass::ApplySettings() {
    SetSetpoint(SETTINGS->TheSettings.Setpoint);
    AUTOPID->ApplySettings();
    AUTOPID->SetAutoTune(1);
    windowStartTime = millis();
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
    
    AUTOPID->Input = SENSOR->GetTemperature();
    AUTOPID->Loop();

    // turn the output pin on/off based on pid output
    unsigned long now = millis();
    if (now - windowStartTime > AUTOPID->WindowSize) {
        //time to shift the Relay Window
        windowStartTime += AUTOPID->WindowSize;
    }
    bool state = ((now - windowStartTime) < AUTOPID->Output);
    BOILER->SetBoilerState(state);
    
    return 0;
}
