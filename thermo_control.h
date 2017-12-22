#ifndef THERMOSTAT_H
#define THERMOSTAT_H

#include <Arduino.h>
#include "settings.h"
#include "sensor.h"
#include "boiler.h"
#include "PID_v1.h"
#include "enumerations.h"

#define THERMOSTAT_MODE_COUNT 5

class ThermostatClass {
public:
    ThermostatClass(PID*, SettingsClass*, SensorClass*, BoilerClass*);
    void Init();
    int Loop();
    void SetMode(ThermostatMode value);
    float ExteriorTemperature;
    ThermostatMode CurrentThermostatMode;

private:
    SettingsClass* SETTINGS;
    SensorClass* SENSOR;
    BoilerClass* BOILER;
    PID* PIDREG;
    unsigned long WindowStartTime;
    bool GetBoilerStateByWindowWidth(float, float, float);
    float LastOutput;
};

#endif // THERMOSTAT_H
