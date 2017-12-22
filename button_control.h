// button_control.h

#ifndef _BUTTON_CONTROL_h
#define _BUTTON_CONTROL_h

#include "Arduino.h"
#include "button.h"
#include "pinout.h"
#include "thermo_control.h"
#include "led_control.h"
#include "oled_display.h"

class ButtonControlClass
{
public:
    ButtonControlClass(ThermostatClass* thermostat, LedControlClass* leds, OledDisplayClass* display);
    void Init();
    void HandlePressedButtons();

protected:
    ThermostatClass* THERM;
    LedControlClass* LEDS;
    OledDisplayClass* DISPLAY;
    bool button1Down, button2Down;
    bool power;
};

#endif
