#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <Arduino.h>
#include "sensor.h"
#include "boiler.h"
#include "thermo_control.h"
#include "enumerations.h"

class LedControlClass {
public:
    LedControlClass(SensorClass* sensor, BoilerClass* boiler, ThermostatClass* thermostat);
    void Init();
    void SetFlash(byte color);
    void SetBlinkingState();
    void SetAnimationState();
    void DrawAll();
    void SetPower(bool value);
    float lastTemp;

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
    void StartAnimation(int direction, int period);
    void FlashEnqueue(byte color);
    byte FlashDequeue();
    void DoFlash(byte color);
    byte flashQueueSize;
    bool power;
};

#endif // LED_CONTROL_H
