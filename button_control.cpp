#include "button_control.h"

ButtonClass BUTTON1(PIN_BUTTON1);
ButtonClass BUTTON2(PIN_BUTTON2);

ButtonControlClass::ButtonControlClass(ThermostatClass* thermostat): THERM(thermostat) { }

void ButtonControlClass::Init()
{
    BUTTON1.Init();
    BUTTON2.Init();
}

void ButtonControlClass::HandlePressedButtons()
{
    int change = 0;
    if (BUTTON1.ButtonHasBeenPressed())
        change = 1;
    if (BUTTON2.ButtonHasBeenPressed())
        change = -1;
    if (change != 0) {
        int newMode = ((int)THERM->GetMode() + change) % THERMOSTAT_MODE_COUNT;
        if (newMode < 0) newMode = THERMOSTAT_MODE_COUNT - 1;
        THERM->SetMode(ThermostatMode(newMode));
        //zunoSendReport(ZUNO_REPORT_SETPOINT);
    }
}
