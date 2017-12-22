/**
 * @brief Main Thermostat class
 *          Gives access to desired temperature, real temperature & humidity
 *
 */

#include "thermo_control.h"

 /**
  * @brief Constructor. Turns off the boiler
  *
  */
ThermostatClass::ThermostatClass(PID* pid, SettingsClass* settings, SensorClass* sensor, BoilerClass* boiler) :
    PIDREG(pid), SETTINGS(settings), SENSOR(sensor), BOILER(boiler) { }

/**
* @brief Initialization, to be called in the Setup() function
*
*/
void ThermostatClass::Init()
{
    BOILER->SetBoilerState(false);
    WindowStartTime = 0;
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
        WindowStartTime = 0;
    }
}

/**
 * @brief Get the desired thermostat mode
 *
 * @return ThermostatMode   the desired thermostat mode
 */
ThermostatMode ThermostatClass::GetMode()
{
    return CurrentThermostatMode;
}

/**
 * @brief Main loop function
 *
 * @return int      0 if OK, -1 in case of error
 */
int ThermostatClass::Loop()
{
    //SENSOR->ReadSensor();
    float temp = SENSOR->Temperature;
    float setPoint = SETTINGS->GetSetPoint(CurrentThermostatMode);

    if (WindowStartTime == 0 || ((millis() - WindowStartTime) > SETTINGS->TheSettings->SampleTime)) {
        //time to shift the Relay Window
        //Serial.println("NW");

        float newOutput = PIDREG->Compute(temp, setPoint);
        if (newOutput != -1)
            LastOutput = newOutput;
        WindowStartTime = millis();
    }
    // output = AUTOPID->Loop(temp);
    bool state = GetBoilerStateByWindowWidth(LastOutput / 1000, temp, setPoint);
    BOILER->SetBoilerState(state);

    return 0;
}

/**
 * @brief Get the current boiler state based on output as PWM value and current time position in the time frame
 *
 * @param output   boiler output [0->1]
 *
 * @return byte
 */
bool ThermostatClass::GetBoilerStateByWindowWidth(const float output, const float temp, const float setPoint)
{
    const unsigned long now = millis();
    const bool state = ((now - WindowStartTime) < (output * SETTINGS->TheSettings->SampleTime))
        && (temp < setPoint);
    /*
    for (int i = 0; i < 2; i++) {
        Serial.print("T: ");
        Serial.print(temp);
        Serial.print(" SP: ");
        Serial.print(setPoint);
        Serial.print(" BSwin: ");
        Serial.print((now - WindowStartTime) * 100 / SETTINGS->TheSettings->SampleTime);
        Serial.print("% O: ");
        Serial.print(output * 100);
        Serial.print("% S:");
        Serial.println(state);
    }
    */

    return state;
}
