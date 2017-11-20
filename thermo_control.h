#ifndef THERMOSTAT_H
#define THERMOSTAT_H

#include <Arduino.h>
#include "settings.h"
#include "sensor.h"
#include "boiler.h"
#include "autopid.h"
#include "hysteresis.h"

#define SWITCH_ON 0xff
#define SWITCH_OFF 0

class ThermostatClass {
public:
    ThermostatClass(AutoPidClass* autoPid, SettingsClass* settings, SensorClass* sensor, BoilerClass* boiler, HysteresisClass* hist);
    int Loop();
    void SetSetpoint(float value);
    float GetSetpoint();

private:
    AutoPidClass* AUTOPID;
    SettingsClass* SETTINGS;
    SensorClass* SENSOR;
    BoilerClass* BOILER;
    HysteresisClass* HYST;
    unsigned long windowStartTime;
    byte GetBoilerStateByWindowWidth(float output);
    float LastOutput;
};

#endif // THERMOSTAT_H
