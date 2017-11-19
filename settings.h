#ifndef E2P_ADDR_H
#define E2P_ADDR_H

#include <Arduino.h>

#define E2P_VERSION_NUMBER      1    // version flag (used to trigger eeprom reset to defauts)
#define E2P_START_ADDRESS       1


#define SAMPLE_TIME 30000 //in ms
//#define SAMPLE_TIME 20 * 60000 //in ms

#define THERMOSTAT_MIN 5.0
#define THERMOSTAT_MAX 25.0
#define THERMOSTAT_DEFAULT 18.0



struct settings_s {
    uint8_t E2PVersionNr;
    float   Setpoint;
    float   Kp;                  // (P)roportional Tuning Parameter
    float   Ki;                  // (I)ntegral Tuning Parameter
    float   Kd;                  // (D)erivative Tuning Parameter    
    float   ATuneStep;
    float   ATuneNoise;
    float   ATuneStartValue;
    unsigned int ATuneLookBack;
    byte    crc8;
};

class SettingsClass {
public:
    SettingsClass();
    void RestoreSettings();
    void PersistSettings();
    settings_s TheSettings;
private:
    byte GetCrc8(byte* data, byte count);
};

#endif // E2P_ADDR_H
