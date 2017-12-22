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
        }
        else
            ButtonHasBeenReleased = true;
    } else {
        ButtonHasBeenPressed = false;
        ButtonHasBeenReleased = false;
    }
}
