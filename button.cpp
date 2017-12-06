#include "button.h"

ButtonClass::ButtonClass(byte pin) {
    Pin = pin;
    ButtonState = HIGH;
}

void ButtonClass::Init() {
    pinMode(Pin, INPUT);
}

bool ButtonClass::ButtonHasBeenPressed() {
    byte reading = digitalRead(Pin);
    if (reading != ButtonState) {
        ButtonState = reading;
        if (ButtonState == LOW) {
            return true;
        }
    }
    return false;
}

