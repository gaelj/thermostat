#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>

class TimerClass {
public:
    TimerClass(unsigned long durationInMillis);
    void Init();
    bool IsElapsed();
    bool IsActive;

private:
    unsigned long DurationInMillis;
    unsigned long StartTime;
};

#endif // TIMER_H
