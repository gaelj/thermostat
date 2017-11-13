/**
 * @brief Main Thermostat class
 *          Gives access to boiler state, desired temperature, real temperature & humidity,
 *          and implements ZWave getter & setter functions
 * 
 */

#include "thermo_control.h"

DHT DhtSensor(PIN_DHT_TEMP_SENSOR, DHT22);

/**
 * @brief Constructor. Does required initialisations and turns the boiler off
 * 
 */
ThermostatClass::ThermostatClass(AutoPidClass* autoPid): AutoPid1(autoPid) {
    SETTINGS.RestoreSettings();
    Input = 18.0;
    Output = 0;
    OutputForWindow = 0;
    Setpoint = SETTINGS.TheSettings.DesiredTemperature;
    windowSize = SAMPLE_TIME;
    unsigned long sampleTime = SAMPLE_TIME;

    DhtSensor.begin();
    byte r[1];
    SetBoilerState(false);
    AutoPid1->SetPointers(&Input, &Output, &Setpoint, windowSize, sampleTime);
    AutoPid1->SetAutoTune(1);
    windowStartTime = millis();
}

/**
 * @brief Set the state of the boiler to on or off
 * 
 * @param value     the desired state
 */
void ThermostatClass::SetBoilerState(bool value) {
    if (currentBoilerState != value) {
        currentBoilerState = value;
        zunoSendToGroupSetValueCommand(CONTROL_GROUP1, value ? SWITCH_ON : SWITCH_OFF);
    }
}

/**
 * @brief Get the current state of the boiler
 * 
 * @return true     the boiler is on
 * @return false    the boiler is off
 */
bool ThermostatClass::GetBoilerState() {
    return currentBoilerState;
}

/**
 * @brief Set the desired room temperature
 * 
 * @param value     the desired temperature
 */
void ThermostatClass::SetDesiredTemperature(float value) {
    if (Setpoint != value) {
        Setpoint = value;
        SETTINGS.TheSettings.DesiredTemperature = value;
        SETTINGS.PersistSettings();
    }
}

/**
 * @brief Get the desired room temperature
 * 
 * @return float   the desired temperature
 */
float ThermostatClass::GetDesiredTemperature() {
    return Setpoint;
}

/**
 * @brief Get the current room temperature
 * 
 * @return float   the room temperature
 */
float ThermostatClass::GetRealTemperature() {
    byte forceRead = 0;
    realTemp = DhtSensor.readTemperature(forceRead);
    return realTemp;
}

/**
 * @brief Get the current room humidity
 * 
 * @return float   the room humidity
 */
float ThermostatClass::GetRealHumidity() {
    byte forceRead = 0;
    realHum = DhtSensor.readHumidity(forceRead);
    return realHum;
}

/**
 * @brief MCU loop function
 * 
 * @return int      0 if OK, -1 in case of error
 */
int ThermostatClass::Loop() {
    Input = GetRealTemperature();
    AutoPid1->Loop();

    /************************************************
     turn the output pin on/off based on pid output
    ************************************************/
    unsigned long now = millis();
    if (now - windowStartTime > windowSize) {
        //time to shift the Relay Window
        windowStartTime += windowSize;
        if (Output < (MIN_CYCLE / 2)) OutputForWindow = 0;
        else if (Output < MIN_CYCLE) OutputForWindow = MIN_CYCLE;
        else if ((windowSize - Output) < (MIN_CYCLE / 2)) OutputForWindow = windowSize;
        else if ((windowSize - Output) < MIN_CYCLE) OutputForWindow = windowSize - MIN_CYCLE;
        else OutputForWindow = Output;
    }
    bool state = ((now - windowStartTime) < OutputForWindow);
    SetBoilerState(state);
    return 0;
}
