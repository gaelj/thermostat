// button_control.h

#ifndef _BUTTON_CONTROL_h
#define _BUTTON_CONTROL_h

#include "Arduino.h"
#include "button.h"
#include "pinout.h"
#include "thermo_control.h"

class ButtonControlClass
{
public:
    ButtonControlClass(ThermostatClass* thermostat);
    void Init();
    void HandlePressedButtons();

protected:
    ThermostatClass* THERM;
};

#endif
