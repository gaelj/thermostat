#include "led.h"

LedClass::LedClass() { }

void LedClass::Init(byte pinR, byte pinG, byte pinB)
{
    pins[0] = pinR;
    pins[1] = pinG;
    pins[2] = pinB;
    for (byte i = 0; i < 3; i++)
        pinMode(pins[i], OUTPUT);
}

void LedClass::DisplayColor(byte color)
{
    for (byte i = 0; i < 3; i++)
        digitalWrite(pins[i], bitRead(color, 2 - i));
}
