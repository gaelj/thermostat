#include "settings.h"

settings_s _Settings;

SettingsClass::SettingsClass()
{
    TheSettings = &_Settings;
}

/**
 * @brief Get the selected setpoint temperature
 * 
 */
float SettingsClass::GetSetPoint(const ThermostatMode mode)
{
    switch (mode) {
        case Frost:  return _Settings.Setpoint_Frost;
        case Absent: return _Settings.Setpoint_Absent;
        case Night:  return _Settings.Setpoint_Night;
        case Day:    return _Settings.Setpoint_Day;
        case Warm:   return _Settings.Setpoint_Warm;
        default:     return _Settings.Setpoint_Absent;
    }
}

/**
* @brief Calculate the CRC8 value of a structure
*
* @param data
* @param count
* @return byte
*/
byte SettingsClass::GetCrc8(byte* data, byte count)
{
    byte result = 0xDF;
    while (count--) {
        result ^= *data;
        data++;
    }
    return result;
}

/**
* @brief Restore the settings struct from E2P. Checks the CRC8 and version values. Resets to default values in case of error
*
* @return true   The settings are valid
* @return false  The settings are invalid (wrong CRC or E2P version has changed)
*/
bool SettingsClass::RestoreSettings()
{
    //Serial.println("Restore stgs");
    EEPROM.get(E2P_START_ADDRESS, &_Settings, sizeof(settings_s));
    DumpSettings();
    return GetCrc8((byte*)&_Settings, sizeof(settings_s) - 1) == _Settings.crc8
            && _Settings.Version == E2P_VERSION;
}

/**
* @brief Apply default values to TheSettings
*
*/
void SettingsClass::LoadDefaults()
{
    //Serial.println("Reset stgs");
    // Invalid data - reset all
    _Settings.Version = E2P_VERSION;
    _Settings.Setpoint_Frost = DEFAULT_Setpoint_Frost;
    _Settings.Setpoint_Absent = DEFAULT_Setpoint_Absent;
    _Settings.Setpoint_Night = DEFAULT_Setpoint_Night;
    _Settings.Setpoint_Day = DEFAULT_Setpoint_Day;
    _Settings.Setpoint_Warm = DEFAULT_Setpoint_Warm;
    _Settings.Kp = DEFAULT_Kp;
    _Settings.Ki = DEFAULT_Ki;
    _Settings.Kd = DEFAULT_Kd;
    _Settings.SampleTime = DEFAULT_SampleTime;
    /*
    TheSettings.HysteresisRange = 0.5;
    TheSettings.ATuneStep = 50;
    TheSettings.ATuneNoise = 1;
    TheSettings.ATuneStartValue = 100;
    TheSettings.ATuneLookBack = 20;
    */
}

/**
* @brief Calculate the CRC8 and persist the settings struct to E2P
*
*/
bool SettingsClass::PersistSettings()
{
    //Serial.print("Persist stgs ");
    //Serial.println((int)sizeof(settings_s));
    _Settings.crc8 = GetCrc8((byte*)&_Settings, sizeof(settings_s) - 1);
    EEPROM.put(E2P_START_ADDRESS, &_Settings, sizeof(settings_s));
    if (!RestoreSettings()) {
        //Serial.println("Persist check error");
        return false;
    }
    return true;
}

/**
* @brief Dump all setting values to Serial
*
*/
void SettingsClass::DumpSettings()
{
    //Serial.println("============ Settings:");
    Serial.println(_Settings.Version);
    Serial.println(_Settings.Setpoint_Frost);
    Serial.println(_Settings.Setpoint_Absent);
    Serial.println(_Settings.Setpoint_Night);
    Serial.println(_Settings.Setpoint_Day);
    Serial.println(_Settings.Setpoint_Warm);
    Serial.println(_Settings.Kp);
    Serial.println(_Settings.Ki);
    Serial.println(_Settings.Kd);
    /*
    Serial.println(TheSettings.HysteresisRange);
    Serial.println(TheSettings.ATuneStep);
    Serial.println(TheSettings.ATuneNoise);
    Serial.println(TheSettings.ATuneStartValue);
    Serial.println(TheSettings.ATuneLookBack);
    */
    Serial.println(_Settings.SampleTime);
    Serial.println(_Settings.crc8);
    //Serial.println("============");
}
