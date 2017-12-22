#ifndef OLEDDISPLAY_H
#define OLEDDISPLAY_H

#include <Arduino.h>

#include "settings.h"
#include "sensor.h"
#include "boiler.h"
#include "thermo_control.h"
#include "enumerations.h"
#include "zwave_encoding.h"

#define OLED_PAGE_COUNT     3

class OledDisplayClass
{
public:
    OledDisplayClass(SettingsClass* settings, SensorClass* sensor,
        BoilerClass* boiler, ThermostatClass* thermostat, PID* pid);
    void Init();
    void DrawDisplay();
    void SetPower(bool value);

private:
    SettingsClass* SETTINGS;
    SensorClass* SENSOR;
    BoilerClass* BOILER;
    ThermostatClass* THERM;
    PID* PIDREG;
    byte lastBoilerState;
    float lastTemp;
    ThermostatMode lastMode;
    byte currentPage;
    bool DisplayRedrawNeeded();
    void ShowNumericValues(float* values, int count);
    void DrawPage(const byte id);
};

#endif // OLEDDISPLAY_H
