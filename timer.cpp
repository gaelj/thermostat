#include "timer.h"

TimerClass::TimerClass(unsigned long durationInMillis)
{
    DurationInMillis = durationInMillis;
    Start();
    IsActive = false;
}

void TimerClass::Start()
{
    // start from the last timer end timestamp if possible
    unsigned long currentDuration = GetCurrentDuration();
    if (currentDuration > DurationInMillis && currentDuration < (DurationInMillis * 2))
        StartTime += DurationInMillis;
    else
        StartTime = millis();
    IsActive = true;
}

bool TimerClass::IsElapsed()
{
    Progress = GetProgress();
    if (!IsActive || GetCurrentDuration() >= DurationInMillis) {
        IsActive = false;
        return true;
    }
    return false;
}

float TimerClass::GetProgress()
{
    unsigned long currentDuration = GetCurrentDuration();
    float currentDurationSec = currentDuration / 1000;
    float totalDurationSec = DurationInMillis / 1000;

    return (currentDurationSec * 100) / totalDurationSec;
}

unsigned long TimerClass::GetCurrentDuration()
{
    return millis() - StartTime;
}
