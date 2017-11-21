#ifndef THERMOSTAT_H
#define THERMOSTAT_H

#include <Arduino.h>
#include "settings.h"
#include "sensor.h"
#include "boiler.h"
#include "autopid.h"
#include "hysteresis.h"
#include "thermostat_mode.h"

#define SWITCH_ON 0xff
#define SWITCH_OFF 0

class ThermostatClass {
public:
    ThermostatClass(AutoPidClass* autoPid, SettingsClass* settings, SensorClass* sensor,
        BoilerClass* boiler, HysteresisClass* hist, ThermostatModeClass* mode);
    int Loop();
    void SetMode(ThermostatMode value);
    ThermostatMode GetMode();

private:
    AutoPidClass* AUTOPID;
    SettingsClass* SETTINGS;
    SensorClass* SENSOR;
    BoilerClass* BOILER;
    HysteresisClass* HYST;
    ThermostatModeClass* MODE;
    unsigned long WindowStartTime;
    byte GetBoilerStateByWindowWidth(float output);
    float LastOutput;
};

#endif // THERMOSTAT_H
