#ifndef HYSTERESIS_H
#define HYSTERESIS_H

#include <Arduino.h>
#include "settings.h"

class HysteresisClass {
public:
    HysteresisClass(settings_s*);
    float Loop(const float, const float);

private:
    settings_s* TheSettings;
    bool heatCycleIsActive;
};

#endif // HYSTERESIS_H
