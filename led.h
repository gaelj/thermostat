#ifndef LED_H
#define LED_H

#include <Arduino.h>

// Colors - RGB format
#define COLOR_BLACK   0b000
#define COLOR_RED     0b100
#define COLOR_GREEN   0b010
#define COLOR_BLUE    0b001
#define COLOR_MAGENTA 0b101
#define COLOR_CYAN    0b011
#define COLOR_YELLOW  0b110
#define COLOR_WHITE   0b111

class LedClass
{
public:
    LedClass();
    void Init(byte pinR, byte pinG, byte pinB);
    void DisplayColor(byte color);

private:
    byte pins[3];
};

#endif // LED_H
