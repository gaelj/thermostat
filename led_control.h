#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <Arduino.h>
#include "pinout.h"
#include "led.h"
#include "timer.h"
#include "thermostat_mode.h"

#define LED_COUNT 3
#define FLASHES 3

class LedControlClass {
public:
    LedControlClass(LedClass* led0, LedClass* led1, LedClass* led2,
        TimerClass* flashTimer, TimerClass* blinkTimer, TimerClass* animationTimer);
    void Init();
    void DisplayColorAll(byte color0, byte color1, byte color2);
    void FlashAll(byte color);
    void SetBlinkingState(bool state);
    void SetAnimation(int direction);
    void DrawAll(ThermostatMode mode);

private:
    LedClass* LED0;
    LedClass* LED1;
    LedClass* LED2;
    TimerClass* FLASH_TIMER;
    TimerClass* BLINK_TIMER;
    TimerClass* ANIMATION_TIMER;
    bool ledBlinkState = false;
    byte ledColor = COLOR_BLACK;
    byte ledColor0 = COLOR_BLACK;
    byte ledColor1 = COLOR_BLACK;
    byte ledColor2 = COLOR_BLACK;
    byte flashColor = COLOR_BLACK;
    byte flashCounter = 0;
    int animationDirection = 0;
    int animationIndex = 0;
};

#endif // LED_CONTROL_H
