#ifndef ENUMERATIONS_H
#define ENUMERATIONS_H

#include <Arduino.h>

#define THERMOSTAT_MODE_COUNT 5

/**
* @brief Possible modes for the thermostat
*
*/
enum ThermostatMode {
    Frost = 0,
    Absent,
    Night,
    Day,
    Warm
};

#define THERMOSTAT_MODE_COUNT 5

#endif // ENUMERATIONS_H
