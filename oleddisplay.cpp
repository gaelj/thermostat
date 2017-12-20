#include "oleddisplay.h"
#include <ZUNO_OLED_I2C.h>
#include <ZUNO_OLED_FONT_NUMB16.h>
#include "icons.h"
#include "timer.h"
#include "settings.h"

OLED SCREEN;
TimerClass SENSOR_TIMER(OLED_SENSOR_PERIOD);
TimerClass PAGE_TIMER(OLED_PAGE_PERIOD);

OledDisplayClass::OledDisplayClass(SettingsClass* settings, SensorClass* sensor,
    BoilerClass* boiler, ThermostatClass* thermostat, PID* pid)
    : SETTINGS(settings), SENSOR(sensor), BOILER(boiler), THERM(thermostat), PIDREG(pid)
{
    lastBoilerState = 1;
    lastMode = Absent;
    currentPage = OLED_PAGE_COUNT - 1;
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
    bool timerElapsed = false;
    if (PAGE_TIMER.IsElapsed()) {
        PAGE_TIMER.Start();
        currentPage = (currentPage + 1) % OLED_PAGE_COUNT;
        timerElapsed = true;
    }

    if (timerElapsed) {
        DisplayRedrawNeeded();
        // clear screen
        SCREEN.clrscr();
        delay(5);
        SCREEN.setFont(zuno_font_numbers16);

        switch (currentPage) {
            case 0: ShowPage_0(); break;
            case 1: ShowPage_1(); break;
        }
    }
}

void OledDisplayClass::ShowPage_0()
{
    // temperatures
    float values[3];
    values[0] = SENSOR->GetTemperature();
    values[1] = THERM->ExteriorTemperature;
    values[2] = SETTINGS->GetSetPoint(THERM->GetMode());

    for (byte i = 0; i < 3; i++) {
        SCREEN.gotoXY(13, 3 * i);
        SCREEN.fixPrint((long)(10 * values[i]), 1);
    }

    // thermostat mode
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
    SCREEN.gotoXY(80, 0);
    SCREEN.writeData(data);

    // boiler state
    if (BOILER->GetBoilerState()) {
        SCREEN.gotoXY(80, 4);
        SCREEN.writeData(flame_data);
    }
}

void OledDisplayClass::ShowPage_1()
{
    // PID values
    float values[3];
    values[0] = PIDREG->lastInput;
    values[1] = PIDREG->lastOutput;
    values[2] = PIDREG->outputSum;

    for (byte i = 0; i < 3; i++) {
        SCREEN.gotoXY(13, 3 * i);
        SCREEN.fixPrint((long)(10 * values[i]), 1);
    }
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
