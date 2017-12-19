#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

/**
* @brief Access to a button
*
*/
class ButtonClass
{
public:
    ButtonClass(byte pin);
    void Init();
    void ReadButton();
    bool ButtonHasBeenPressed;
    bool ButtonHasBeenReleased;
    byte ButtonState;

private:
    byte Pin;
};

#endif // BUTTON_H
