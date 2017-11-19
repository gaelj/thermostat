#ifndef BOILER_H
#define BOILER_H

#include <Arduino.h>
#include <ZUNO_legacy_channels.h>
#include <ZUNO_channels.h>
#include <ZUNO_Definitions.h>

#define SWITCH_ON 0xff
#define SWITCH_OFF 0

#define BOILER_MIN_TIME      60000 // 1mn min time between boiler state changes

#define CONTROL_GROUP_1      1  // Boiler group

class BoilerClass {
public:
    BoilerClass();
    void SetBoilerState(bool value);
    bool GetBoilerState();

private:
    bool currentBoilerState;
    unsigned long lastBoilerChange;
};

#endif // BOILER_H
