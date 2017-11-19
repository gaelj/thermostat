/**
 * @brief Main Thermostat class
 *          Gives access to boiler state, desired temperature, real temperature & humidity,
 *          and implements ZWave getter & setter functions
 * 
 */

#include "thermo_control.h"

#ifdef TEMP_DHT
DHT DhtSensor(PIN_DHT_TEMP_SENSOR, DHT22);
#endif
#ifdef TEMP_DS18B20
OneWire ow(BUS_PIN);
DS18B20Sensor ds18b20(&ow);
byte sensor_roms[ROM_SIZE*MAX_SENSOR];
#define ROM_DATA(index) (&sensor_roms[index*ROM_SIZE])
#endif

/**
 * @brief Constructor. Does required initialisations and turns the boiler off
 * 
 */
ThermostatClass::ThermostatClass(AutoPidClass* autoPid, SettingsClass* settings): AUTOPID(autoPid), SETTINGS(settings) {
    Input = 18.0;
    Output = 0;
    OutputForWindow = 0;
#ifdef TEMP_DHT
    DhtSensor.begin();
#endif
#ifdef TEMP_DS18B20
    ds18b20.findAllSensors(sensor_roms);
#endif
    SetBoilerState(false);
}

/**
 * @brief Apply the settings
 * 
 */
void ThermostatClass::ApplySettings() {
    SetSetpoint(SETTINGS->TheSettings.Setpoint);
    windowSize = SAMPLE_TIME;
    unsigned long sampleTime = SAMPLE_TIME;
    AUTOPID->ApplySettings(&Input, &Output, &Setpoint, windowSize, sampleTime);
    AUTOPID->SetAutoTune(1);
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
        zunoSendToGroupSetValueCommand(CONTROL_GROUP_1, value > 0 ? SWITCH_ON : SWITCH_OFF);
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
void ThermostatClass::SetSetpoint(float value) {
    if (value < THERMOSTAT_MIN) value = THERMOSTAT_MIN;
    else if (value > THERMOSTAT_MAX) value = THERMOSTAT_MAX;
    if (Setpoint != value) {
        Setpoint = value;
        SETTINGS->TheSettings.Setpoint = value;
        SETTINGS->PersistSettings();
    }
}

/**
 * @brief Get the desired room temperature
 * 
 * @return float   the desired temperature
 */
float ThermostatClass::GetSetpoint() {
    return Setpoint;
}

void ThermostatClass::ReadRealTemperature() {
    byte forceRead = 0;
#ifdef TEMP_DHT
    realTemp = DhtSensor.readTemperature(forceRead);
#endif
#ifdef TEMP_DS18B20
    realTemp = ds18b20.getTemperature(ROM_DATA(0));
#endif
    Serial.println(realTemp);
}
/**
 * @brief Get the current room temperature
 * 
 * @return float   the room temperature
 */
float ThermostatClass::GetRealTemperature() {
    return realTemp;
}

/**
 * @brief Get the current room humidity
 * 
 * @return float   the room humidity
 */
float ThermostatClass::GetRealHumidity() {
    byte forceRead = 0;
#ifdef TEMP_DHT
    realHum = DhtSensor.readHumidity(forceRead);
#endif
#ifdef TEMP_DS18B20
    realHum = 0;
#endif
    return realHum;
}

/**
 * @brief MCU loop function
 * 
 * @return int      0 if OK, -1 in case of error
 */
int ThermostatClass::Loop() {
    ReadRealTemperature();
    /*
    Input = GetRealTemperature();
    AUTOPID->Loop();

    // turn the output pin on/off based on pid output
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
    */
    return 0;
}
