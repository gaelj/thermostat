#include "settings.h"
#include <EEPROM.h>

/**
 * @brief Constructor
 * 
 */
SettingsClass::SettingsClass() {}

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
 */
void SettingsClass::RestoreSettings() {
    EEPROM.get(E2P_START_ADDRESS, &TheSettings, sizeof(settings_s));
    // Check data
    if (GetCrc8((byte*)&TheSettings, sizeof(settings_s) - 1) != TheSettings.crc8 || TheSettings.E2PVersionNr != E2P_VERSION_NUMBER) {
        // Invalid data - reset all
        TheSettings.E2PVersionNr = E2P_VERSION_NUMBER;
        TheSettings.DesiredTemperature = 18.0;
        TheSettings.Kp = 2;
        TheSettings.Ki = 0.5;
        TheSettings.Kd = 2;
        TheSettings.ATuneStep = 50;
        TheSettings.ATuneNoise = 1;
        TheSettings.ATuneStartValue = 100;
        TheSettings.ATuneLookBack = 20;
        PersistSettings();
    }
}

/**
 * @brief Calculate the CRC8 and persist the settings struct to E2P
 * 
 */
void SettingsClass::PersistSettings() {
    TheSettings.crc8 = GetCrc8((byte*)&TheSettings, sizeof(settings_s) - 1);
    EEPROM.put(E2P_START_ADDRESS, &TheSettings, sizeof(settings_s));
}
