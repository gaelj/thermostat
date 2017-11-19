#ifndef THERMOSTAT_H
#define THERMOSTAT_H

#define TEMP_DS18B20
#undef  TEMP_DHT

#include <Arduino.h>
#include <ZUNO_legacy_channels.h>
#include <ZUNO_channels.h>
#include <ZUNO_Definitions.h>

#ifdef TEMP_DHT
#include <ZUNO_DHT.h>
#endif
#ifdef TEMP_DS18B20
#define BUS_PIN 9
#define ROM_SIZE 8
#define MAX_SENSOR 1
#include <ZUNO_OneWire.h>
#include <ZUNO_DS18B20.h>
#endif

#include <PID_v1.h>
#include <PID_AutoTune_v0.h>
#include "settings.h"
#include "pinout.h"
#include "autopid.h"

#define SAMPLE_TIME 30000 //in ms
//#define SAMPLE_TIME 20 * 60000 //in ms
#define MIN_CYCLE    3 * 60000 //in ms

#define SWITCH_ON 0xff
#define SWITCH_OFF 0

#define CONTROL_GROUP_1      1  // Boiler group

#define THERMOSTAT_MIN 5.0
#define THERMOSTAT_MAX 25.0

class ThermostatClass {
public:
    ThermostatClass(AutoPidClass* autoPid, SettingsClass* settings);
    void ApplySettings();
    int Loop();
    void SetBoilerState(bool value);
    bool GetBoilerState();
    void SetSetpoint(float value);
    float GetSetpoint();
    float GetRealTemperature();
    float GetRealHumidity();
    void ReadRealTemperature();

private:
    AutoPidClass* AUTOPID;
    SettingsClass* SETTINGS;
    bool currentBoilerState;
    float realTemp;
    float realHum;
    float Setpoint;
    float Input;
    float Output;
    float windowSize;
    unsigned long windowStartTime;
    float OutputForWindow;
};

#endif // THERMOSTAT_H
