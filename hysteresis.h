#ifndef HYSTERESIS_H
#define HYSTERESIS_H

#include <Arduino.h>
#include "settings.h"

class HysteresisClass {
public:
    HysteresisClass(SettingsClass* settings);
    float Loop(const float);

private:
    SettingsClass* SETTINGS;
};

#endif // HYSTERESIS_H
