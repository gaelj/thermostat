#include "oled_display.h"
#include <ZUNO_OLED_I2C.h>
#include <ZUNO_OLED_FONT_NUMB16.h>
#include "icons.h"
#include "timer.h"
#include "settings.h"

OLED SCREEN;
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
    lastTemp = SENSOR->Temperature;
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

    if (DisplayRedrawNeeded() || timerElapsed) {
        SCREEN.clrscr();
        delay(5);
        SCREEN.setFont(zuno_font_numbers16);
        DrawPage(currentPage);
    }
}

/**
* @brief Draw page by id
*
*/
void OledDisplayClass::DrawPage(const byte id)
{
    float values[3];
    char* data = absent_data;

    switch (id) {
        case 0:
            // temperatures
            values[0] = SENSOR->Temperature;
            values[1] = THERM->ExteriorTemperature;
            values[2] = SETTINGS->GetSetPoint(THERM->CurrentThermostatMode);

            // thermostat mode icon
            switch (THERM->CurrentThermostatMode) {
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

            // boiler state icon
            if (BOILER->CurrentBoilerState) {
                SCREEN.gotoXY(80, 4);
                SCREEN.writeData(flame_data);
            }
            break;
        case 1:
            // PID values
            values[0] = PIDREG->lastInput;
            values[1] = PIDREG->lastOutput;
            values[2] = PIDREG->outputSum;
            break;
        case 2:
            // PID values
            values[0] = PIDREG->error;
            values[1] = PIDREG->dInput;
            values[2] = SENSOR->Humidity;
            break;
    }
    ShowNumericValues(values, 3);
}

/**
* @brief Display a maxmimum of 3 float values in a column
*
* @param values Array of float values
* @param count size of the array
*/
void OledDisplayClass::ShowNumericValues(float* values, int count)
{
    for (byte i = 0; i < count; i++) {
        SCREEN.gotoXY(13, 3 * i);
        SCREEN.fixPrint((long)(10 * values[i]), 1);
    }
}

/**
* @brief Set the OLED display power on or off
*
*/
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
    // Refresh display if temperature has changed
    // Refresh display if thermostat mode has changed
    // Refresh display if boiler state has changed
    if (SENSOR->Temperature != lastTemp ||
        THERM->CurrentThermostatMode != lastMode ||
        BOILER->CurrentBoilerState != lastBoilerState) {

        lastTemp = SENSOR->Temperature;
        lastMode = THERM->CurrentThermostatMode;
        lastBoilerState = BOILER->CurrentBoilerState;
        return true;
    }
    return false;
}
