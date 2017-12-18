#ifndef OLEDDISPLAY_H
#define OLEDDISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <ZUNO_OLED_I2C.h>
#include <ZUNO_OLED_FONT_NUMB16.h>

#include "settings.h"
#include "sensor.h"
#include "boiler.h"
#include "thermo_control.h"
#include "thermostat_mode.h"

class DisplayClass
{
public:
    DisplayClass(SettingsClass* settings, SensorClass* sensor,
        BoilerClass* boiler, ThermostatClass* thermostat, ThermostatModeClass* mode);
    void Init();
    void DrawDisplay();

private:
    SettingsClass* SETTINGS;
    SensorClass* SENSOR;
    BoilerClass* BOILER;
    ThermostatClass* THERM;
    ThermostatModeClass* MODE;
};

#endif // OLEDDISPLAY_H
