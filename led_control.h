#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <Arduino.h>
#include "sensor.h"
#include "boiler.h"
#include "thermo_control.h"

#define LED_COUNT       3
#define FLASHES         3 // 0-1-0
#define FLASH_QUEUE_LEN 32

#define BOILER_BLINK_COLOR      COLOR_RED
#define SET_SETPOINT_COLOR      COLOR_BLUE
#define GET_SETPOINT_COLOR      COLOR_YELLOW
#define GET_TEMPRATURE_COLOR    COLOR_GREEN
#define ZUNO_CALLBACK_COLOR     COLOR_WHITE

class LedControlClass {
public:
    LedControlClass(SensorClass* sensor, BoilerClass* boiler, ThermostatClass* thermostat);
    void Init();
    void SetFlash(byte color);
    void SetBlinkingState();
    void SetAnimationState();
    void DrawAll();
    void SetPower(bool value);

private:
    SensorClass* SENSOR;
    BoilerClass* BOILER;
    ThermostatClass* THERM;
    bool ledBlinkState;
    byte ledColors[LED_COUNT];
    byte flashColor;
    byte flashCounter;
    byte flashQueue[FLASH_QUEUE_LEN];
    int animationDirection;
    int animationIndex;
    float lastTemp;
    void DisplayColorAll();
    void StartAnimation(int direction, int period);
    void FlashEnqueue(byte color);
    byte FlashDequeue();
    void DoFlash(byte color);
    byte flashQueueSize;
    bool power;
};

#endif // LED_CONTROL_H
