#ifndef THERM_MODE_H
#define THERM_MODE_H

#include <Arduino.h>

#define THERMOSTAT_MODE_COUNT 5

/**
* @brief Possible modes for the thermostat
*
*/
enum ThermostatMode {
    Frost,
    Absent,
    Night,
    Day,
    Warm
};


/**
* @brief Access to the settings
* 
*/
class ThermostatModeClass {
public:
    ThermostatModeClass();
    ThermostatMode CurrentThermostatMode;
    byte Encode(ThermostatMode mode);
    ThermostatMode Decode(byte code);
};

#endif // THERM_MODE_H
