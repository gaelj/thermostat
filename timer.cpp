#include "timer.h"

TimerClass::TimerClass(unsigned long durationInMillis) {
    DurationInMillis = durationInMillis;
    Init();
    IsActive = false;
}

void TimerClass::Init() {
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
