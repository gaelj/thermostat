#ifndef OLEDDISPLAY_H
#define OLEDDISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <ZUNO_OLED_I2C.h>
#include <ZUNO_OLED_FONT_NUMB16.h>

#include "settings.h"
#include "timer.h"
#include "sensor.h"
#include "boiler.h"
#include "thermo_control.h"
#include "thermostat_mode.h"

class OledDisplayClass
{
public:
    OledDisplayClass(SettingsClass* settings, SensorClass* sensor,
        BoilerClass* boiler, ThermostatClass* thermostat, ThermostatModeClass* mode);
    void Init();
    void DrawDisplay();
    bool DisplayRedrawNeeded();

private:
    SettingsClass* SETTINGS;
    SensorClass* SENSOR;
    BoilerClass* BOILER;
    ThermostatClass* THERM;
    ThermostatModeClass* MODE;
    byte lastBoilerState;
    float lastTemp;
    ThermostatMode lastMode;
};

#endif // OLEDDISPLAY_H
