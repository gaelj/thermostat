#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <Arduino.h>
#include "pinout.h"
#include "led.h"
#include "timer.h"
#include "thermostat_mode.h"
#include "sensor.h"
#include "boiler.h"
#include "thermo_control.h"

#define LED_COUNT 3
#define FLASHES 3

class LedControlClass {
public:
    LedControlClass(SensorClass* sensor, BoilerClass* boiler, ThermostatClass* thermostat);
    void Init();
    void DisplayColorAll(byte color0, byte color1, byte color2);
    void FlashAll(byte color);
    void SetBlinkingState();
    void SetAnimationState();
    void StartAnimation(int direction, int period);
    void DrawAll();

private:
    SensorClass* SENSOR;
    BoilerClass* BOILER;
    ThermostatClass* THERM;
    bool ledBlinkState;
    byte ledColor;
    byte ledColor0;
    byte ledColor1;
    byte ledColor2;
    byte flashColor;
    byte flashCounter;
    int animationDirection;
    int animationIndex;
    int lastTemp;
};

#endif // LED_CONTROL_H
