#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>

class TimerClass {
public:
    TimerClass(unsigned long durationInMillis);
    void Start();
    bool IsElapsed();
    bool IsActive;
    unsigned long DurationInMillis;
    float GetProgressPercentage();

private:
    unsigned long StartTime;
};

#endif // TIMER_H
