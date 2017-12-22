/**
 * @brief Boiler class
 *          Gives access to boiler state
 * 
 */

#include "boiler.h"

TimerClass SAFETY_TIMER(BOILER_MIN_TIME);

/**
 * @brief Constructor. Does required initialisations and turns the boiler off
 * 
 */
BoilerClass::BoilerClass()
{
    CurrentBoilerState = SWITCH_OFF;
}

/**
 * @brief Set the state of the boiler to on or off, unless the last state change was too recent
 * 
 * @param value     the desired state
 */
void BoilerClass::SetBoilerState(byte value)
{
    if (CurrentBoilerState != value && SAFETY_TIMER.IsElapsed()) {
        CurrentBoilerState = value;
        zunoSendToGroupSetValueCommand(CONTROL_GROUP_1, value);
        SAFETY_TIMER.Start();
    }
}
