#ifndef OLEDDISPLAY_H
#define OLEDDISPLAY_H

#include <Arduino.h>

#include "settings.h"
#include "sensor.h"
#include "boiler.h"
#include "thermo_control.h"
#include "thermostat_mode.h"

class OledDisplayClass
{
public:
    OledDisplayClass(SettingsClass* settings, SensorClass* sensor,
        BoilerClass* boiler, ThermostatClass* thermostat);
    void Init();
    void DrawDisplay();
    bool DisplayRedrawNeeded();
    void SetPower(bool value);

private:
    SettingsClass* SETTINGS;
    SensorClass* SENSOR;
    BoilerClass* BOILER;
    ThermostatClass* THERM;
    byte lastBoilerState;
    float lastTemp;
    ThermostatMode lastMode;
};

#endif // OLEDDISPLAY_H
