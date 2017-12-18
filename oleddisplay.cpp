#include "oleddisplay.h"
#include "icons.h"

OLED SCREEN;
TimerClass SENSOR_TIMER(10000);

OledDisplayClass::OledDisplayClass(SettingsClass* settings, SensorClass* sensor,
        BoilerClass* boiler, ThermostatClass* thermostat, ThermostatModeClass* mode)
        : SETTINGS(settings), SENSOR(sensor),
            BOILER(boiler), THERM(thermostat), MODE(mode)
{
    lastBoilerState = 1;
    lastMode = Absent;
    lastTemp = SENSOR->GetTemperature();
}

/**
* @brief Initalize the screen
*
*/
void OledDisplayClass::Init()
{
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

/**
* @brief Should the display be redrawn, due to source data update
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

