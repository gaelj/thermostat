#ifndef AUTOPID_H
#define AUTOPID_H

#include <Arduino.h>
#include <PID_v1.h>
#include <PID_AutoTune_v0.h>
#include "settings.h"

class AutoPidClass {
public:
    AutoPidClass(PID* pid, PID_ATune* atune, SettingsClass* settings);
    void ApplySettings(float* Input, float* Output, float* Setpoint, float WindowSize, unsigned long sampleTime);
    void Loop();
    void SetAutoTune(char b);

    float kp, ki, kd;
    float aTuneStep, aTuneNoise, aTuneStartValue;
    unsigned int aTuneLookBack;

private:
    PID* myPID;
    PID_ATune* aTune;
    SettingsClass* SETTINGS;
    void ChangeAutoTune();
    void AutoTuneHelper(bool start);
    void SerialPrintInfoString();
    bool tuning;
    unsigned long serialTime;
    byte ATuneModeRemember;
};

#endif // AUTOPID_H
