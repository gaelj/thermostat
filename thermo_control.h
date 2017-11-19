#ifndef THERMOSTAT_H
#define THERMOSTAT_H

#include <Arduino.h>
#include <PID_v1.h>
#include <PID_AutoTune_v0.h>
#include "settings.h"
#include "pinout.h"
#include "sensor.h"
#include "boiler.h"
#include "autopid.h"

#define SWITCH_ON 0xff
#define SWITCH_OFF 0

class ThermostatClass {
public:
    ThermostatClass(AutoPidClass* autoPid, SettingsClass* settings, SensorClass* sensor, BoilerClass* boiler);
    void ApplySettings();
    int Loop();
    void SetSetpoint(float value);
    float GetSetpoint();

private:
    AutoPidClass* AUTOPID;
    SettingsClass* SETTINGS;
    SensorClass* SENSOR;
    BoilerClass* BOILER;
    unsigned long windowStartTime;
};

#endif // THERMOSTAT_H
