/**
 * @brief Boiler class
 *          Gives access to boiler state
 * 
 */

#include "boiler.h"

/**
 * @brief Constructor. Does required initialisations and turns the boiler off
 * 
 */
BoilerClass::BoilerClass()
{
    lastBoilerChange = 0;
    currentBoilerState = 0;
}

/**
 * @brief Set the state of the boiler to on or off, unless the last state change was too recent
 * 
 * @param value     the desired state
 */
void BoilerClass::SetBoilerState(bool value)
{
    if (currentBoilerState != value && 
            (lastBoilerChange == 0 ||
                ((millis() - lastBoilerChange) >= BOILER_MIN_TIME))) {
        currentBoilerState = value;
        zunoSendToGroupSetValueCommand(CONTROL_GROUP_1, value > 0 ? SWITCH_ON : SWITCH_OFF);
        lastBoilerChange = millis();
    }
}

/**
 * @brief Get the current state of the boiler
 * 
 * @return true     the boiler is on
 * @return false    the boiler is off
 */
bool BoilerClass::GetBoilerState()
{
    return currentBoilerState;
}
