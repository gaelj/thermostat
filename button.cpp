#include "button.h"

ButtonClass::ButtonClass(byte pin)
{
    Pin = pin;
    ButtonState = HIGH;
    ButtonHasBeenPressed = false;
    ButtonHasBeenReleased = false;
}

void ButtonClass::Init()
{
    pinMode(Pin, INPUT);
}

void ButtonClass::ReadButton()
{
    byte reading = digitalRead(Pin);
    if (reading != ButtonState) {
        ButtonState = reading;
        if (ButtonState == LOW) {
            ButtonHasBeenPressed = true;
            return;
        }
        if (ButtonState == HIGH) {
            ButtonHasBeenReleased = true;
            return;
        }
    }
    ButtonHasBeenPressed = false;
    ButtonHasBeenReleased = false;
}
