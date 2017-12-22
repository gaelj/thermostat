#ifndef THERMOSTAT_H
#define THERMOSTAT_H

#include <Arduino.h>
#include "settings.h"
#include "sensor.h"
#include "boiler.h"
#include "PID_v1.h"
#include "enumerations.h"
#include "timer.h"

class ThermostatClass {
public:
    ThermostatClass(PID*, SettingsClass*, SensorClass*, BoilerClass*);
    void Init();
    int Loop();
    void SetMode(ThermostatMode value);
    float ExteriorTemperature;
    ThermostatMode CurrentThermostatMode;
    TimerClass* BOILER_TIMER;
    TimerClass* PID_TIMER;

private:
    SettingsClass* SETTINGS;
    SensorClass* SENSOR;
    BoilerClass* BOILER;
    PID* PIDREG;
    float LastOutput;
};

#endif // THERMOSTAT_H
