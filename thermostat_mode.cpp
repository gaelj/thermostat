#include "settings.h"

/**
* @brief Constructor
*
*/
ThermostatModeClass::ThermostatModeClass() {
    CurrentThermostatMode = Frost;
}

/**
* @brief Convert a thermostat mode to byte value for Zwave slider report
*
*/
byte ThermostatModeClass::Encode(const ThermostatMode mode) {
    return float(mode) * 25; // = ThermostatMode length - 1
}

/**
* @brief Convert a byte value to thermostat mode for Zwave slider control
*
*/
ThermostatMode ThermostatModeClass::Decode(byte code) {
    if (code == 99) code = 100;
    Serial.println(code);
    Serial.println(code / 25);
    return ThermostatMode(code / 25);
}
