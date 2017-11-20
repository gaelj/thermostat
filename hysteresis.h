#ifndef HYSTERESIS_H
#define HYSTERESIS_H

#include <Arduino.h>
#include "settings.h"

class HysteresisClass {
public:
    HysteresisClass(SettingsClass* settings);
    unsigned long Loop(float);

private:
    SettingsClass* SETTINGS;
    float Input;
    unsigned long Output;
    void SerialPrintInfoString();
};

#endif // HYSTERESIS_H
