// the is the INO sketch

#include <ZUNO_legacy_channels.h>
#include <ZUNO_channels.h>
#include <ZUNO_Definitions.h>
#include <ZUNO_DHT.h>
#include <EEPROM.h>
#include <PID_v1.h>
#include <PID_AutoTune_v0.h>

#include "thermo_control.h"
#include "settings.h"

// For some cases use UART (Serial0/Serial1)
// It's a most comfortable way for debugging
#define MY_SERIAL Serial

ZUNO_SETUP_DEBUG_MODE(DEBUG_ON);

// Setup associations - we have 1 group for boiler on/off behavior
ZUNO_SETUP_ASSOCIATIONS(ZUNO_ASSOCIATION_GROUP_SET_VALUE);

// Setup channels - we have 1 channel to get/set the desired temperature,
//                          1 channel to get the real temperature,
//                          1 channel to get the real humidity,
//                          1 channel to get/set boiler state
ZUNO_SETUP_CHANNELS(ZUNO_SWITCH_MULTILEVEL(DesiredTemperatureGetter, DesiredTemperatureSetter),
                    ZUNO_SENSOR_MULTILEVEL_TEMPERATURE(RealTemperatureGetter),
                    ZUNO_SENSOR_MULTILEVEL_HUMIDITY(RealHumidityGetter),
                    ZUNO_SWITCH_BINARY(BoilerGetter, BoilerSetter));

SettingsClass SETTINGS;
PID pid(&SETTINGS);
PID_ATune atune;
AutoPidClass AUTOPID(&pid, &atune, &SETTINGS);
ThermostatClass THERM(&AUTOPID, &SETTINGS);

void setup() {
    MY_SERIAL.begin(115200);
    SETTINGS.RestoreSettings();
    THERM.ApplySettings();
}

bool first = true;

void loop() {
    MY_SERIAL.println("\n **** Loop ****\n");
    THERM.Loop();
    delay(SAMPLE_TIME);
}

/**
 * @brief Zwave Setter for Desired Temperature
 * 
 */
void DesiredTemperatureSetter(byte value) {
    float d = toFloat(value);
    THERM.SetDesiredTemperature(d);
}

/**
 * @brief Zwave Getter for Desired Temperature
 * 
 */
byte DesiredTemperatureGetter() {
    return fromFloat(THERM.GetDesiredTemperature());
}

/**
 * @brief Zwave Setter for Boiler state
 * 
 */
void BoilerSetter(byte value) {
    bool d = (value > 0);
    THERM.SetBoilerState(d);
}

/**
 * @brief Zwave Getter for Boiler state
 * 
 */
byte BoilerGetter() {
    return (byte)(THERM.GetBoilerState() ? 255 : 0);
}

/**
 * @brief Zwave Getter for Real Temperature
 * 
 */
byte RealTemperatureGetter() {
    return fromFloat(THERM.GetRealTemperature());
}

/**
 * @brief Zwave Getter for Real Humidity
 * 
 */
byte RealHumidityGetter() {
    return fromFloat(THERM.GetRealHumidity());
}
