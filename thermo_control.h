#ifndef THERMOSTAT_H
#define THERMOSTAT_H

#include <Arduino.h>
#include <ZUNO_legacy_channels.h>
#include <ZUNO_channels.h>
#include <ZUNO_Definitions.h>
#include <ZUNO_DHT.h>
#include <PID_v1.h>
#include <PID_AutoTune_v0.h>
#include "settings.h"
#include "pinout.h"
#include "utilities.h"
#include "autopid.h"

#define SAMPLE_TIME 1000 //in ms
//#define SAMPLE_TIME 20 * 60000 //in ms
#define MIN_CYCLE    3 * 60000 //in ms

#define SWITCH_ON 0xff
#define SWITCH_OFF 0

#define CONTROL_GROUP1      1  // Boiler group

class ThermostatClass {
public:
    ThermostatClass(AutoPidClass* autoPid, SettingsClass* settings);
    void ApplySettings();
    int Loop();
    void SetBoilerState(bool value);
    bool GetBoilerState();
    void SetDesiredTemperature(float value);
    float GetDesiredTemperature();
    float GetRealTemperature();
    float GetRealHumidity();

private:
    AutoPidClass* AUTOPID;
    SettingsClass* SETTINGS;
    bool currentBoilerState;
    float realTemp;
    float realHum;
    float Input;
    float Output;
    float Setpoint;
    float windowSize;
    unsigned long windowStartTime;
    float OutputForWindow;
};

#endif // THERMOSTAT_H
