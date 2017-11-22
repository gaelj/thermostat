/**
 * @brief Thermostat on Z-uino
 * 
 */

#include <ZUNO_legacy_channels.h>
#include <ZUNO_channels.h>
#include <ZUNO_Definitions.h>

#include "sensor.h"
#ifdef TEMP_DHT
#include <ZUNO_DHT.h>
#endif
#ifdef TEMP_DS18B20
#include <ZUNO_OneWire.h>
#include <ZUNO_DS18B20.h>
#endif

#include <EEPROM.h>
#include <PID_v1.h>
#include <PID_AutoTune_v0.h>

#include "settings.h"
#include "boiler.h"
#include "hysteresis.h"
#include "thermo_control.h"
#include "thermostat_mode.h"

#define MY_SERIAL Serial

#define ZUNO_SENSOR_MULTILEVEL_TEMPERATURE_WORD(GETTER)    ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE, SENSOR_MULTILEVEL_SCALE_CELSIUS, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_TWO_DECIMALS, GETTER)	

ZUNO_SETUP_DEBUG_MODE(DEBUG_ON);

// Setup channels - we have 1 channel to get/set the desired temperature,
//                          1 channel to get the real temperature,
//                          1 channel to get the real humidity
ZUNO_SETUP_CHANNELS(ZUNO_SWITCH_MULTILEVEL(ZGetSetpoint, ZSetSetpoint)
                   ,ZUNO_SENSOR_MULTILEVEL_TEMPERATURE_WORD(ZGetRealTemperature)
                   //,ZUNO_SENSOR_MULTILEVEL_HUMIDITY(RealHumidityGetter)
                    );

// Setup associations - we have 1 group for boiler on/off behavior
ZUNO_SETUP_ASSOCIATIONS(ZUNO_ASSOCIATION_GROUP_SET_VALUE);


SettingsClass SETTINGS;
ThermostatModeClass MODE;
PID pid(&SETTINGS);
PID_ATune atune;
HysteresisClass HIST(&SETTINGS);
SensorClass SENSOR;
BoilerClass BOILER;
AutoPidClass AUTOPID(&pid, &atune, &SETTINGS, &MODE);
ThermostatClass THERM(&AUTOPID, &SETTINGS, &SENSOR, &BOILER, &HIST, &MODE);

float lastTemp = 200;
byte lastBoiler = 1;
ThermostatMode lastMode = Absent;
unsigned long lastZwaveRefresh;
unsigned long const zaveRefreshDelay = 30000;
bool settingsError = false;

/**
* @brief Main setup function
*
*/
void setup() {
    MY_SERIAL.begin(115200);
    if (!SETTINGS.RestoreSettings()) {
        SETTINGS.LoadDefaults();
        if (!SETTINGS.PersistSettings())
            settingsError = true;
    }

    lastZwaveRefresh = 0;
    //AUTOPID.ApplySettings();
    //AUTOPID.SetAutoTune(1);
}

/**
* @brief Main loop function
*
*/
void loop() {
    if (!settingsError) {
        THERM.Loop();

        if (millis() > lastZwaveRefresh + zaveRefreshDelay || lastZwaveRefresh == 0) {
            lastZwaveRefresh = millis();
            MY_SERIAL.println("ZR");
            if (THERM.GetMode() != lastMode || lastZwaveRefresh == 0) {
                MY_SERIAL.println("SP");
                lastMode = THERM.GetMode();
                zunoSendReport(1); // report setpoint
            }
            if (SENSOR.GetTemperature() != lastTemp || lastZwaveRefresh == 0) {
                MY_SERIAL.println("T");
                lastTemp = SENSOR.GetTemperature();
                zunoSendReport(2); // report temperature
            }
        }
    }
    else {
        MY_SERIAL.println("Stg er");
    }

    delay(1000);
    /*
    MY_SERIAL.print("Wait");
    while (millis() - loopStart < SAMPLE_TIME) {
        delay(1000);
        MY_SERIAL.print(".");
    }
    MY_SERIAL.println();
    */
}


/**
 * @brief Zwave Setter for Desired Temperature
 * 
 */
void ZSetSetpoint(byte value) {
    //MY_SERIAL.print("ZSetSetpoint to ");
    //MY_SERIAL.println(value);
    Serial.print("Sp=");
    switch (MODE.Decode(value)) {
        case Frost: Serial.println("Frost"); break;
        case Absent: Serial.println("Absent"); break;
        case Night: Serial.println("Night"); break;
        case Day: Serial.println("Day"); break;
        case Warm: Serial.println("Warm"); break;
    }
    THERM.SetMode(MODE.Decode(value));
}

/**
 * @brief Zwave Getter for Desired Temperature
 * 
 */
byte ZGetSetpoint() {
    //MY_SERIAL.print("ZGetSetpoint ");
    //MY_SERIAL.println((byte)THERM.GetMode());
    return (byte)MODE.Encode(THERM.GetMode());
}

/**
 * @brief Zwave Getter for Real Temperature
 * 
 */
word ZGetRealTemperature() {
    //MY_SERIAL.print("ZGetRealTemp ");
    word temp = (word)(SENSOR.GetTemperature() * 100);
    //MY_SERIAL.println(temp);
    return temp;
}

/**
 * @brief Zwave Getter for Real Humidity
 * 
 *//*
byte RealHumidityGetter() {
    return fromFloat(THERM.GetHumidity());
}
*/

/**
 * @brief Universal handler for all the channels
 * 
 * @remark See callback_data variable 
 *         We use word params for all 
 *         We use zero based index of the channel instead of typical 
 *         Getter/Setter index of Z-Uno. 
 *         See enum ZUNO_CHANNEL*_GETTER/ZUNO_CHANNEL*_SETTER in ZUNO_Definitions.h 
 *
 * @param  
 */
void zunoCallback(void) {
    //MY_SERIAL.println();
    //MY_SERIAL.print("Callback type ");
    //MY_SERIAL.println(callback_data.type);
    switch(callback_data.type) {
        case ZUNO_CHANNEL1_GETTER: callback_data.param.bParam = ZGetSetpoint(); break;
        case ZUNO_CHANNEL1_SETTER: ZSetSetpoint(callback_data.param.bParam); break;
        case ZUNO_CHANNEL2_GETTER: callback_data.param.wParam = ZGetRealTemperature(); break;
        default: break;
    }
}
