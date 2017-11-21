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
ThermostatClass::ThermostatClass(AutoPidClass* autoPid, SettingsClass* settings, SensorClass* sensor,
        BoilerClass* boiler, HysteresisClass* hist, ThermostatModeClass* mode) :
    AUTOPID(autoPid), SETTINGS(settings), SENSOR(sensor), BOILER(boiler), HYST(hist), MODE(mode) {
    BOILER->SetBoilerState(false);
    WindowStartTime = 0;
    LastOutput = 0;
    MODE->CurrentThermostatMode = Absent;
}

/**
 * @brief Set the desired thermostat mode
 *
 * @param value     the desired thermostat mode
 */
void ThermostatClass::SetMode(ThermostatMode value) {
    if (MODE->CurrentThermostatMode != value) {
        MODE->CurrentThermostatMode = value;
        WindowStartTime = 0;
    }
}

/**
 * @brief Get the desired thermostat mode
 *
 * @return ThermostatMode   the desired thermostat mode
 */
ThermostatMode ThermostatClass::GetMode() {
    return MODE->CurrentThermostatMode;
}

/**
 * @brief Main loop function
 *
 * @return int      0 if OK, -1 in case of error
 */
int ThermostatClass::Loop() {
    SENSOR->ReadSensor();

    float temp = SENSOR->GetTemperature();

    if (WindowStartTime == 0 || ((millis() - WindowStartTime) > SETTINGS->TheSettings.SampleTime)) {
        //time to shift the Relay Window
        Serial.println("New window **");
        LastOutput = HYST->Loop(temp, SETTINGS->GetSetPoint(MODE->CurrentThermostatMode));
        WindowStartTime = millis();
    }
    // output = AUTOPID->Loop(temp);
    byte state = GetBoilerStateByWindowWidth(LastOutput);
    BOILER->SetBoilerState(state);

    return 0;
}

/**
 * @brief Get the current boiler state based on autopid output
 *
 * @param output   boiler output [0->1]
 *
 * @return byte
 */
byte ThermostatClass::GetBoilerStateByWindowWidth(const float output) {
    const unsigned long now = millis();
    const bool state = ((now - WindowStartTime) < (output * SETTINGS->TheSettings.SampleTime));

    Serial.print("Get boiler state");
    Serial.print(" window: ");
    Serial.print((now - WindowStartTime) * 100 / SETTINGS->TheSettings.SampleTime);
    Serial.print("% output: ");
    Serial.print(output * 100);
    Serial.print("% state:");
    Serial.println(state);

    return state;
}
