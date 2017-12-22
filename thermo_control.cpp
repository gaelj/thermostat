/**
 * @brief Main Thermostat class
 *          Gives access to desired temperature, real temperature & humidity
 *
 */

#include "thermo_control.h"

TimerClass _PID_TIMER(0);
TimerClass _BOILER_TIMER(BOILER_MIN_TIME);

 /**
  * @brief Constructor. Turns off the boiler
  *
  */
ThermostatClass::ThermostatClass(PID* pid, SettingsClass* settings, SensorClass* sensor, BoilerClass* boiler) :
    PIDREG(pid), SETTINGS(settings), SENSOR(sensor), BOILER(boiler)
{
    BOILER_TIMER = &_BOILER_TIMER;
    PID_TIMER = &_PID_TIMER;
}

/**
* @brief Initialization, to be called in the Setup() function
*
*/
void ThermostatClass::Init()
{
    _PID_TIMER.DurationInMillis = SETTINGS->TheSettings->SampleTime;
    BOILER->SetBoilerState(SWITCH_OFF);
    LastOutput = 0;
    CurrentThermostatMode = Absent;
    ExteriorTemperature = 10;
    PIDREG->Create(0, 1000);
    PIDREG->SetMode(AUTOMATIC);
}

/**
 * @brief Set the desired thermostat mode
 *
 * @param value     the desired thermostat mode
 */
void ThermostatClass::SetMode(ThermostatMode value)
{
    if (CurrentThermostatMode != value) {
        CurrentThermostatMode = value;
        _PID_TIMER.Start();
    }
}

/**
 * @brief Main loop function
 *
 * @return int      0 if OK, -1 in case of error
 */
int ThermostatClass::Loop()
{
    float temp = SENSOR->Temperature;
    float setPoint = SETTINGS->GetSetPoint(CurrentThermostatMode);

    if (_PID_TIMER.IsElapsed()) {
        _PID_TIMER.Start();
        float newOutput = PIDREG->Compute(temp, setPoint);
        if (newOutput != -1)
            LastOutput = newOutput;
        if (LastOutput > 0) {
            _BOILER_TIMER.DurationInMillis = (LastOutput / 1000) * SETTINGS->TheSettings->SampleTime;
            _BOILER_TIMER.Start();
        }
    }
    BOILER->SetBoilerState((!_BOILER_TIMER.IsElapsed() && (temp < setPoint)) ? SWITCH_ON : SWITCH_OFF);

    return 0;
}
