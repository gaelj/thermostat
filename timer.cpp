#include "timer.h"

TimerClass::TimerClass(unsigned long durationInMillis) {
    DurationInMillis = durationInMillis;
    Init();
    IsActive = false;
}

void TimerClass::Init() {
    // start from the last timer end timestamp if possible
    if (millis() > StartTime + DurationInMillis && millis() < StartTime + (DurationInMillis * 2))
        StartTime += DurationInMillis;
    else
        StartTime = millis();
    IsActive = true;
}

bool TimerClass::IsElapsed() {
    if (!IsActive || millis() > StartTime + DurationInMillis) {
        IsActive = false;
        return true;
    }
    return false;
}
