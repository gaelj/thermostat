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
ThermostatClass::ThermostatClass(SettingsClass* settings, SensorClass* sensor,
        BoilerClass* boiler, HysteresisClass* hist, ThermostatModeClass* mode) :
        SETTINGS(settings), SENSOR(sensor), BOILER(boiler), HYST(hist), MODE(mode) {
    BOILER->SetBoilerState(false);
    WindowStartTime = 0;
    LastOutput = 0;
    MODE->CurrentThermostatMode = Absent;
    ExteriorTemperature = 10;
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
    //SENSOR->ReadSensor();

    float temp = SENSOR->GetTemperature();
    float setPoint = SETTINGS->GetSetPoint(MODE->CurrentThermostatMode);

    if (WindowStartTime == 0 || ((millis() - WindowStartTime) > SETTINGS->TheSettings->SampleTime)) {
        //time to shift the Relay Window
        //Serial.println("NW");
        LastOutput = HYST->Loop(temp, setPoint);
        WindowStartTime = millis();
    }
    // output = AUTOPID->Loop(temp);
    bool state = GetBoilerStateByWindowWidth(LastOutput, temp, setPoint);
    BOILER->SetBoilerState(state);

    return 0;
}

/**
 * @brief Get the current boiler state based on output as PWM value and current time position in the time frame
 *
 * @param output   boiler output [0->1]
 *
 * @return byte
 */
bool ThermostatClass::GetBoilerStateByWindowWidth(const float output, const float temp, const float setPoint) {
    const unsigned long now = millis();
    const bool state = ((now - WindowStartTime) < (output * SETTINGS->TheSettings->SampleTime))
        && (temp < setPoint);
    /*
    for (int i = 0; i < 2; i++) {
        Serial.print("T: ");
        Serial.print(temp);
        Serial.print(" SP: ");
        Serial.print(setPoint);
        Serial.print(" BSwin: ");
        Serial.print((now - WindowStartTime) * 100 / SETTINGS->TheSettings->SampleTime);
        Serial.print("% O: ");
        Serial.print(output * 100);
        Serial.print("% S:");
        Serial.println(state);
    }
    */

    return state;
}

/**
* @brief Encode a real temperature to a value from 0 to 100
*
* @param temp   real temperature (float). Precision of 0.5°C. Must be between -25°C and +24°C (100 values)
*
* @return byte ranging from 0 to 100
*/
byte ThermostatClass::EncodeTemperature(float temp)
{
    int value = round((temp + 25) * 2);
    if (value < 0) value = 0;
    if (value > 99) value = 99;
    return (byte)value;
}

/**
* @brief Decode a byte value from 0 to 100 to a real temperature
*
* @param encoded   encoded temperature (byte)
*
* @return float
*/
float ThermostatClass::DecodeTemperature(byte encoded)
{
    return (((float)encoded) / 2) - 25;
}
