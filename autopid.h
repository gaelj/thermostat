#ifndef AUTOPID_H
#define AUTOPID_H

#include <Arduino.h>
#include <PID_v1.h>
#include <PID_AutoTune_v0.h>
#include "settings.h"

class AutoPidClass {
public:
    AutoPidClass(PID* pid, PID_ATune* atune, SettingsClass* settings);
    void ApplySettings();
    float Loop(float);
    void SetAutoTune(char b);

private:
    PID* myPID;
    PID_ATune* aTune;
    SettingsClass* SETTINGS;
    float Input;
    float Output;
    void ChangeAutoTune();
    void AutoTuneHelper(bool start);
    void SerialPrintInfoString();
    bool tuning;
    byte ATuneModeRemember;
};

#endif // AUTOPID_H
