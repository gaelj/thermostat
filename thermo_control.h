#ifndef THERMOSTAT_H
#define THERMOSTAT_H

#include <Arduino.h>
#include "settings.h"
#include "sensor.h"
#include "boiler.h"
#include "hysteresis.h"
#include "thermostat_mode.h"

class ThermostatClass {
public:
    ThermostatClass(SettingsClass* settings, SensorClass* sensor,
        BoilerClass* boiler, HysteresisClass* hist, ThermostatModeClass* mode);
    int Loop();
    void SetMode(ThermostatMode value);
    ThermostatMode GetMode();
    byte EncodeTemperature(float temp);
    float DecodeTemperature(byte encoded);
    float ExteriorTemperature;

private:
    SettingsClass* SETTINGS;
    SensorClass* SENSOR;
    BoilerClass* BOILER;
    HysteresisClass* HYST;
    ThermostatModeClass* MODE;
    unsigned long WindowStartTime;
    bool GetBoilerStateByWindowWidth(float, float, float);
    float LastOutput;
};

#endif // THERMOSTAT_H
