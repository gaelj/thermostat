#include "zwave_encoding.h"


/**
* @brief Convert a thermostat mode to byte value for Zwave slider report
*
*/
byte EncodeMode(const ThermostatMode mode)
{
    return (mode + 1) * 10;
}

/**
* @brief Convert a byte value to thermostat mode for Zwave slider control
*
*/
ThermostatMode DecodeMode(const byte code)
{
    return ThermostatMode((code / 10) - 1);
}

/**
* @brief Encode a real temperature to a value from 0 to 100
*
* @param temp   real temperature (float). Precision of 0.5°C. Must be between -25°C and +24°C (100 values)
*
* @return byte ranging from 0 to 100
*/
byte EncodeExteriorTemperature(const float temp)
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
float DecodeExteriorTemperature(const byte encoded)
{
    return (((float)encoded) / 2) - 25;
}

/**
* @brief Convert a temperature mode to byte value for Zwave report
*
*/
word EncodeSensorReading(const float value)
{
    return (word)(value * 100);
}
