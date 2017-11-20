#ifndef E2P_ADDR_H
#define E2P_ADDR_H

#include <Arduino.h>
#include <EEPROM.h>

#define E2P_VERSION             1
#define E2P_START_ADDRESS       1

//#define SAMPLE_TIME 30000 //in ms
#define SAMPLE_TIME 20 * 60000 //in ms

#define THERMOSTAT_MIN 5.0
#define THERMOSTAT_MAX 25.0
#define THERMOSTAT_DEFAULT 18.0

#define BOILER_MIN_TIME      3 * 60000 // 3mn min time between boiler state changes

struct settings_s {
    byte Version;
    float Setpoint;
    float Kp;                  // (P)roportional Tuning Parameter
    float Ki;                  // (I)ntegral Tuning Parameter
    float Kd;                  // (D)erivative Tuning Parameter
    unsigned long SampleTime;
    float HysteresisRange;
    float ATuneStep;
    float ATuneNoise;
    float ATuneStartValue;
    float ATuneLookBack;
    byte  crc8;
};

class SettingsClass {
public:
    SettingsClass();
    bool RestoreSettings();
    bool PersistSettings();
    void LoadDefaults();
    void DumpSettings();
    settings_s TheSettings;
private:
    byte GetCrc8(byte* data, byte count);
};

#endif // E2P_ADDR_H
