#include "oleddisplay.h"
#include "icons.h"

OLED SCREEN;

DisplayClass::DisplayClass(SettingsClass* settings, SensorClass* sensor,
        BoilerClass* boiler, ThermostatClass* thermostat, ThermostatModeClass* mode)
        : SETTINGS(settings), SENSOR(sensor),
            BOILER(boiler), THERM(thermostat), MODE(mode) { }

/**
* @brief Initalize the screen
*
*/
void DisplayClass::Init()
{
    SCREEN.begin();
    SCREEN.clrscr();
}

/**
* @brief Draw the screen
*
*/
void DisplayClass::DrawDisplay()
{
    // clear screen
    SCREEN.clrscr();
    delay(5);

    // sensor temperature
    SCREEN.setFont(zuno_font_numbers16);
    SCREEN.gotoXY(13, 0);
    SCREEN.fixPrint((long)(10 * SENSOR->GetTemperature()), 1);

    // setpoint temperature
    SCREEN.gotoXY(13, 4);
    SCREEN.fixPrint((long)(10 * SETTINGS->GetSetPoint(MODE->CurrentThermostatMode)), 1);

    // boiler state
    if (BOILER->GetBoilerState()) {
        SCREEN.gotoXY(80, 4);
        SCREEN.writeData(flame_data);
    }

    // thermostat mode
    SCREEN.gotoXY(80, 0);
    switch (THERM->GetMode()) {
        case Frost:
            SCREEN.writeData(snow_data);
            SCREEN.setBrightness(0x00);
            break;
        case Absent:
            SCREEN.writeData(absent_data);
            SCREEN.setBrightness(0x00);
            break;
        case Night:
            SCREEN.writeData(moon_data);
            SCREEN.setBrightness(0x00);
            break;
        case Day:
            SCREEN.writeData(sun_data);
            SCREEN.setBrightness(0xFF);
            break;
        case Warm:
            SCREEN.writeData(hot_data);
            SCREEN.setBrightness(0xFF);
            break;
    }
}
