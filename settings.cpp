#include "settings.h"

/**
* @brief Constructor. Sets the selected setpoint to Absent
*
* @param theSettings
*/
SettingsClass::SettingsClass(settings_s* theSettings): TheSettings(theSettings) { }

/**
 * @brief Get the selected setpoint temperature
 * 
 */
float SettingsClass::GetSetPoint(const ThermostatMode mode) {
    switch (mode) {
        case Frost: return TheSettings->Setpoint_Frost;
        case Absent: return TheSettings->Setpoint_Absent;
        case Night: return TheSettings->Setpoint_Night;
        case Day: return TheSettings->Setpoint_Day;
        case Warm: return TheSettings->Setpoint_Warm;
        default: return TheSettings->Setpoint_Absent;
    }
}

/**
* @brief Calculate the CRC8 value of a structure
*
* @param data
* @param count
* @return byte
*/
byte SettingsClass::GetCrc8(byte* data, byte count) {
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
bool SettingsClass::RestoreSettings() {
    Serial.println("Restore stgs");
    EEPROM.get(E2P_START_ADDRESS, &TheSettings, sizeof(settings_s));
    DumpSettings();
    if (GetCrc8((byte*)&TheSettings, sizeof(settings_s) - 1) != TheSettings->crc8 || TheSettings->Version != E2P_VERSION) {
        return false;
    }
    return true;
}

/**
* @brief Apply default values to TheSettings
*
*/
void SettingsClass::LoadDefaults() {
    Serial.println("Reset stgs");
    // Invalid data - reset all
    TheSettings->Version = E2P_VERSION;
    TheSettings->Setpoint_Frost = 5.0;
    TheSettings->Setpoint_Absent = 14.0;
    TheSettings->Setpoint_Night = 15.0;
    TheSettings->Setpoint_Day = 19.0;
    TheSettings->Setpoint_Warm = 22.0;
    TheSettings->Kp = 2;
    TheSettings->Ki = 0.5;
    TheSettings->Kd = 2;
    TheSettings->SampleTime = 20 * 60000; // 30000
    TheSettings->HysteresisRange = 0.5;
    TheSettings->ATuneStep = 50;
    TheSettings->ATuneNoise = 1;
    TheSettings->ATuneStartValue = 100;
    TheSettings->ATuneLookBack = 20;
}

/**
* @brief Calculate the CRC8 and persist the settings struct to E2P
*
*/
bool SettingsClass::PersistSettings() {
    Serial.print("Persist stgs ");
    Serial.println((int)sizeof(settings_s));

    TheSettings->crc8 = GetCrc8((byte*)&TheSettings, sizeof(settings_s) - 1);
    EEPROM.put(E2P_START_ADDRESS, &TheSettings, sizeof(settings_s));
    if (!RestoreSettings()) {
        Serial.println("Persist check error");
        return false;
    }
    return true;
}

/**
* @brief Dump all setting values to Serial
*
*/
void SettingsClass::DumpSettings() {
    Serial.println("============ Settings:");
    Serial.println(TheSettings->Version);
    Serial.println(TheSettings->Setpoint_Frost);
    Serial.println(TheSettings->Setpoint_Absent);
    Serial.println(TheSettings->Setpoint_Night);
    Serial.println(TheSettings->Setpoint_Day);
    Serial.println(TheSettings->Setpoint_Warm);
    Serial.println(TheSettings->Kp);
    Serial.println(TheSettings->Ki);
    Serial.println(TheSettings->Kd);
    Serial.println(TheSettings->ATuneStep);
    Serial.println(TheSettings->ATuneNoise);
    Serial.println(TheSettings->ATuneStartValue);
    Serial.println(TheSettings->ATuneLookBack);
    Serial.println(TheSettings->SampleTime);
    Serial.println(TheSettings->HysteresisRange);
    Serial.println(TheSettings->crc8);
    Serial.println("============");
}
