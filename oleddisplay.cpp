#include "oleddisplay.h"
#include "icons.h"
#include "timer.h"
#include <ZUNO_OLED_I2C.h>
#include <ZUNO_OLED_FONT_NUMB16.h>

OLED SCREEN;
TimerClass SENSOR_TIMER(10000);

OledDisplayClass::OledDisplayClass(SettingsClass* settings, SensorClass* sensor,
        BoilerClass* boiler, ThermostatClass* thermostat)
        : SETTINGS(settings), SENSOR(sensor),
            BOILER(boiler), THERM(thermostat)
{
    lastBoilerState = 1;
    lastMode = Absent;
}

/**
* @brief Initalize the screen
*
*/
void OledDisplayClass::Init()
{
    lastTemp = SENSOR->GetTemperature();
    SCREEN.begin();
    SCREEN.clrscr();
}

/**
* @brief Draw the screen
*
*/
void OledDisplayClass::DrawDisplay()
{
    // clear screen
    SCREEN.clrscr();
    delay(5);

    // sensor temperature
    SCREEN.setFont(zuno_font_numbers16);
    SCREEN.gotoXY(13, 0);
    SCREEN.fixPrint((long)(10 * SENSOR->GetTemperature()), 1);

    // exterior temperature
    SCREEN.gotoXY(13, 3);
    SCREEN.fixPrint((long)(10 * THERM->ExteriorTemperature), 1);

    // setpoint temperature
    SCREEN.gotoXY(13, 6);
    SCREEN.fixPrint((long)(10 * SETTINGS->GetSetPoint(THERM->GetMode())), 1);

    // boiler state
    if (BOILER->GetBoilerState()) {
        SCREEN.gotoXY(80, 4);
        SCREEN.writeData(flame_data);
    }

    // thermostat mode
    SCREEN.gotoXY(80, 0);
    char* data = absent_data;
    switch (THERM->GetMode()) {
        case Frost:
            data = snow_data;
            break;
        //case Absent:
        //    data = absent_data;
        //    break;
        case Night:
            data = moon_data;
            break;
        case Day:
            data = sun_data;
            break;
        case Warm:
            data = hot_data;
            break;
    }
    SCREEN.writeData(data);
}

void OledDisplayClass::SetPower(bool value)
{
    if (value)
        SCREEN.on();
    else
        SCREEN.off();
}


/**
* @brief Should the display be redrawn, due to source data update. Reads the temperature sensor
* 
*/
bool OledDisplayClass::DisplayRedrawNeeded()
{
    // Refresh display if thermostat mode has changed
    bool drawDisplay = false;
    if (THERM->GetMode() != lastMode) {
        lastMode = THERM->GetMode();
        drawDisplay = true;
    }

    // Refresh display if temperature has changed
    if (SENSOR_TIMER.IsElapsed()) {
        SENSOR_TIMER.Start();
        SENSOR->ReadSensor();
        if (SENSOR->GetTemperature() != lastTemp) {
            lastTemp = SENSOR->GetTemperature();
            drawDisplay = true;
        }
    }

    // Refresh display if boiler state has changed
    if (BOILER->GetBoilerState() != lastBoilerState) {
        lastBoilerState = BOILER->GetBoilerState();
        drawDisplay = true;
    }
    return drawDisplay;
}

