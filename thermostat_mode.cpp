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
    return (mode + 1) * 10;
}

/**
* @brief Convert a byte value to thermostat mode for Zwave slider control
*
*/
ThermostatMode ThermostatModeClass::Decode(byte code) {
    return ThermostatMode((code / 10) - 1);
}
