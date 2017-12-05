#include "led.h"

LedClass::LedClass(byte pinR, byte pinG, byte pinB) {
    PinR = pinR;
    PinG = pinG;
    PinB = pinB;
}

void LedClass::Begin() {
    pinMode(PinR, OUTPUT);
    pinMode(PinG, OUTPUT);
    pinMode(PinB, OUTPUT);
    DisplayColor(COLOR_BLACK);
}


void LedClass::DisplayColor(byte color) {
    digitalWrite(PinR, bitRead(color, 2));
    digitalWrite(PinG, bitRead(color, 1));
    digitalWrite(PinB, bitRead(color, 0));
}
