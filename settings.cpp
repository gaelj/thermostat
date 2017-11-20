#include "settings.h"

/**
 * @brief Constructor
 * 
 */
SettingsClass::SettingsClass() { }

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
    if (GetCrc8((byte*)&TheSettings, sizeof(settings_s) - 1) != TheSettings.crc8 || TheSettings.Version != E2P_VERSION) {
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
    TheSettings.Version = E2P_VERSION;
    TheSettings.Setpoint = 17.5;
    TheSettings.Kp = 2;
    TheSettings.Ki = 0.5;
    TheSettings.Kd = 2;
    TheSettings.SampleTime = SAMPLE_TIME;
    TheSettings.HysteresisRange = 0.5;
    TheSettings.ATuneStep = 50;
    TheSettings.ATuneNoise = 1;
    TheSettings.ATuneStartValue = 100;
    TheSettings.ATuneLookBack = 20;
}

/**
 * @brief Calculate the CRC8 and persist the settings struct to E2P
 * 
 */
void SettingsClass::PersistSettings() {
    Serial.print("Persist stgs ");
    Serial.println((int)sizeof(settings_s));

    TheSettings.crc8 = GetCrc8((byte*)&TheSettings, sizeof(settings_s) - 1);
    EEPROM.put(E2P_START_ADDRESS, &TheSettings, sizeof(settings_s));
    if (!RestoreSettings()) {
        Serial.println("Persist check error");
    }
}

/**
 * @brief Dump all setting values to Serial
 * 
 */
void SettingsClass::DumpSettings() {
    Serial.println("============ Settings:");
    Serial.println(TheSettings.Version);
    Serial.println(TheSettings.Setpoint);
    Serial.println(TheSettings.Kp);
    Serial.println(TheSettings.Ki);
    Serial.println(TheSettings.Kd);
    Serial.println(TheSettings.ATuneStep);
    Serial.println(TheSettings.ATuneNoise);
    Serial.println(TheSettings.ATuneStartValue);
    Serial.println(TheSettings.ATuneLookBack);
    Serial.println(TheSettings.SampleTime);
    Serial.println(TheSettings.HysteresisRange);
    Serial.println(TheSettings.crc8);
    Serial.println("============");
}
