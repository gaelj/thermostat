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
    if ((millis() - StartTime) > DurationInMillis && (millis() - StartTime) < (DurationInMillis * 2))
        StartTime += DurationInMillis;
    else
        StartTime = millis();
    IsActive = true;
}

bool TimerClass::IsElapsed()
{
    if (!IsActive || (millis() - StartTime) >= DurationInMillis) {
        IsActive = false;
        return true;
    }
    return false;
}

float TimerClass::GetProgressPercentage()
{
    float n = (float)((millis() - StartTime) / 1000);
    float d = (float)(DurationInMillis / 1000);
    return n / d;
}
