#include "button_control.h"

ButtonClass BUTTON1(PIN_BUTTON1);
ButtonClass BUTTON2(PIN_BUTTON2);

ButtonControlClass::ButtonControlClass(ThermostatClass* thermostat, LedControlClass* leds, OledDisplayClass* display):
    THERM(thermostat), LEDS(leds), DISPLAY(display)
{
    button1Down = false;
    button2Down = false;
    power = true;
}

void ButtonControlClass::Init()
{
    BUTTON1.Init();
    BUTTON2.Init();
}

void ButtonControlClass::HandlePressedButtons()
{
    int change = 0;
    BUTTON1.ReadButton();
    BUTTON2.ReadButton();
    if (BUTTON1.ButtonHasBeenPressed)
        button1Down = true;
    if (BUTTON2.ButtonHasBeenPressed)
        button2Down = true;

    if (button1Down && button2Down) {
        // double press
        power = !power;
        LEDS->SetPower(power);
        DISPLAY->SetPower(power);
        button1Down = false;
        button2Down = false;
    }
    else if (BUTTON1.ButtonHasBeenReleased && button1Down) {
        change = 1;
        button1Down = false;
    }
    else if (BUTTON2.ButtonHasBeenReleased && button2Down) {
        change = -1;
        button2Down = false;
    }

    if (change != 0) {
        int newMode = ((int)THERM->GetMode() + change) % THERMOSTAT_MODE_COUNT;
        if (newMode < 0) newMode = THERMOSTAT_MODE_COUNT - 1;
        THERM->SetMode(ThermostatMode(newMode));
        //zunoSendReport(ZUNO_REPORT_SETPOINT);
    }
}
