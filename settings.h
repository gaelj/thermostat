#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <EEPROM.h>
#include "thermostat_mode.h"

#define E2P_VERSION             2 // change this value to apply default settings on first boot after flash
#define E2P_START_ADDRESS       1

#define THERMOSTAT_MIN          5.0        // minimum thermostat setting
#define THERMOSTAT_MAX          25.0       // maximum thermostat setting
#define THERMOSTAT_DEFAULT      18.0   // default thermostat setting

#define BOILER_MIN_TIME         1 * 60000 // 3mn min time between boiler state changes


/**
 * @brief Structure containing all settings persisted to EEPROM
 *
 */
struct settings_s {
    byte Version;
    float Setpoint_Frost;
    float Setpoint_Absent;
    float Setpoint_Night;
    float Setpoint_Day;
    float Setpoint_Warm;
    float Kp;                  // (P)roportional Tuning Parameter
    float Ki;                  // (I)ntegral Tuning Parameter
    float Kd;                  // (D)erivative Tuning Parameter
    unsigned long SampleTime;  // The time between 2 measurements
    /*
    float HysteresisRange;     // The number of degrees below setpoint at which heating is set
    float ATuneStep;
    float ATuneNoise;
    float ATuneStartValue;
    float ATuneLookBack;
    */
    byte  crc8;
};

/**
 * @brief Access to the settings
 */
class SettingsClass {
public:
    SettingsClass(settings_s* theSettings);
    bool RestoreSettings();
    bool PersistSettings();
    void LoadDefaults();
    void DumpSettings();
    settings_s* TheSettings;
    float GetSetPoint(const ThermostatMode mode);

private:
    byte GetCrc8(byte* data, byte count);
};

#endif // SETTINGS_H
