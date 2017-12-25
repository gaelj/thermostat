/**
* @brief Intelligent thermostat implementation on Z-uno platform
*
*/

#include "button_control.h"
#include "pinout.h"
#include "sensor.h"

#include <Print.h>
#include <string.h>
#include <EEPROM.h>
#include <Wire.h>
#include <ZUNO_legacy_channels.h>
#include <ZUNO_channels.h>
#include <ZUNO_Definitions.h>
#include <ZUNO_OLED_I2C.h>
#include <ZUNO_OLED_FONT_NUMB16.h>

#ifdef TEMP_DHT
#include <ZUNO_DHT.h>
#endif
#ifdef TEMP_DS18B20
#include <ZUNO_OneWire.h>
#include <ZUNO_DS18B20.h>
#endif

#include "string_builder.h"
#include "PID_v1.h"
//#include "PID_AutoTune_v0.h"
#include "settings.h"
#include "enumerations.h"
#include "boiler.h"
#include "thermo_control.h"
#include "zwave_encoding.h"
#include "led.h"
#include "led_control.h"
#include "timer.h"
#include "button.h"
#include "button_control.h"
#include "oled_display.h"

#define MY_SERIAL Serial

// Custom sensor to retrieve temperature as a word (2 bytes)
#define ZUNO_SENSOR_MULTILEVEL_TEMPERATURE_WORD(GETTER) ZUNO_SENSOR_MULTILEVEL (ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE, SENSOR_MULTILEVEL_SCALE_CELSIUS, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_TWO_DECIMALS, GETTER)
#define ZUNO_SENSOR_MULTILEVEL_HUMIDITY_WORD(GETTER)    ZUNO_SENSOR_MULTILEVEL (ZUNO_SENSOR_MULTILEVEL_TYPE_RELATIVE_HUMIDITY, SENSOR_MULTILEVEL_SCALE_PERCENTAGE_VALUE, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_TWO_DECIMALS, GETTER)

ZUNO_SETUP_DEBUG_MODE(DEBUG_ON);

// Defines the order of the Zwave channels
#define ZUNO_REPORT_SETPOINT    1
#define ZUNO_REPORT_EXT_TEMP    2
#define ZUNO_REPORT_TEMP        3
#define ZUNO_REPORT_HUMIDITY    4

// Zwave channels: 1 channel to get/set the desired temperature,
//                 1 channel to (get/)set the exterior temperature,
//                 1 channel to get the real temperature,
//                 1 channel to get the real humidity
ZUNO_SETUP_CHANNELS(ZUNO_SWITCH_MULTILEVEL(ZGetSetpoint, ZSetSetpoint)
    , ZUNO_SWITCH_MULTILEVEL(ZGetExteriorTemperature, ZSetExteriorTemperature)
    , ZUNO_SENSOR_MULTILEVEL_TEMPERATURE_WORD(ZGetRealTemperature)
    , ZUNO_SENSOR_MULTILEVEL_HUMIDITY_WORD(ZGetRealHumidity)
);

// Zwave associations: 1 group to set boiler relay on/off
// (association to the relay must be setup in the Zwave network master)
ZUNO_SETUP_ASSOCIATIONS(ZUNO_ASSOCIATION_GROUP_SET_VALUE);

// Create objects
TimerClass ZWAVE_TIMER(ZWAVE_PERIOD);
TimerClass SENSOR_TIMER(OLED_SENSOR_PERIOD);
SettingsClass SETTINGS;
SensorClass SENSOR;
BoilerClass BOILER;

PID PIDREG(&SETTINGS);
/*
PID_ATune atune;
AutoPidClass AUTOPID(&pid, &atune, &SETTINGS, &MODE);
ThermostatClass THERM(&SETTINGS, &SENSOR, &BOILER, &HIST, &MODE);
*/
ThermostatClass THERM(&PIDREG, &SETTINGS, &SENSOR, &BOILER);
LedControlClass LEDS(&SENSOR, &BOILER, &THERM);
OledDisplayClass DISPLAY(&SETTINGS, &SENSOR, &BOILER, &THERM, &PIDREG, &LEDS);
ButtonControlClass BUTTONS(&THERM, &LEDS, &DISPLAY);

/**
* @brief Main setup function
*
*/
void setup()
{
    MY_SERIAL.begin(115200);
    //if (!SETTINGS.RestoreSettings()) {
    //SETTINGS.LoadDefaults();
    //if (!SETTINGS.PersistSettings())
    //    settingsError = true;
    //}

    //AUTOPID.ApplySettings();
    //AUTOPID.SetAutoTune(1);
}

/**
* @brief Main loop function
*
*/
void loop()
{
    unsigned long loopStart = millis();

    if (SENSOR_TIMER.IsElapsed()) {
        SENSOR_TIMER.Start();
        SENSOR.ReadSensor();
    }

    // Handle button presses
    BUTTONS.ReadButtons();

    // Set LED blinking if boiler is on
    LEDS.SetBlinkingState();

    // Set LED animation if temperature has changed
    LEDS.SetAnimationState();

    // Refresh LED states
    LEDS.DrawAll();

    // Refresh OLED display
    DISPLAY.DrawDisplay();

    // Run the thermostat loop
    if (ZWAVE_TIMER.IsElapsed()) {
        ZWAVE_TIMER.Start();
        THERM.Loop();
        zunoSendReport(ZUNO_REPORT_SETPOINT);
        zunoSendReport(ZUNO_REPORT_EXT_TEMP);
        zunoSendReport(ZUNO_REPORT_TEMP);
        zunoSendReport(ZUNO_REPORT_HUMIDITY);
    }

    // Wait if needed
    unsigned long loopTime = millis() - loopStart;
    delay(LOOP_DELAY > loopTime ? LOOP_DELAY - loopTime : 1);
}


/**
* @brief Zwave Getter for Desired Temperature
*
*/
byte ZGetSetpoint()
{
    LEDS.SetFlash(ZUNO_CALLBACK_COLOR);
    return EncodeMode(THERM.CurrentThermostatMode);
}

/**
* @brief Zwave Setter for Desired Temperature
*
*/
void ZSetSetpoint(byte value)
{
    if (THERM.CurrentThermostatMode != DecodeMode(value)) {
        LEDS.SetFlash(SET_SETPOINT_COLOR);
        THERM.SetMode(DecodeMode(value));
        //zunoSendReport(ZUNO_REPORT_SETPOINT);
    }
}

/**
* @brief Zwave Getter for external Temperature
*
*/
byte ZGetExteriorTemperature()
{
    //LEDS.SetFlash(GET_SETPOINT_COLOR);
    return EncodeExteriorTemperature(THERM.ExteriorTemperature);
}

/**
* @brief Zwave Setter for external Temperature
*
*/
void ZSetExteriorTemperature(byte value)
{
    if (THERM.ExteriorTemperature != DecodeExteriorTemperature(value)) {
        //LEDS.SetFlash(SET_SETPOINT_COLOR);
        THERM.ExteriorTemperature = DecodeExteriorTemperature(value);
        //zunoSendReport(ZUNO_REPORT_SETPOINT);
    }
}

/**
* @brief Zwave Getter for Real Temperature
*
*/
word ZGetRealTemperature()
{
    //LEDS.SetFlash(GET_TEMPRATURE_COLOR);
    return EncodeSensorReading(SENSOR.Temperature);
}

/**
* @brief Zwave Getter for Real Humidity
*
*/
byte ZGetRealHumidity()
{
    return EncodeSensorReading(SENSOR.Humidity);
}

/**
* @brief Universal handler for all the channels
*
* @remark See callback_data variable
*         We use word params for all
*         We use zero based index of the channel instead of typical
*         Getter/Setter index of Z-Uno.
*         See enum ZUNO_CHANNEL*_GETTER/ZUNO_CHANNEL*_SETTER in ZUNO_Definitions.h
*/
void zunoCallback(void)
{
    //LEDS.SetFlash(ZUNO_CALLBACK_COLOR);
    switch (callback_data.type) {
        case ZUNO_CHANNEL1_GETTER: callback_data.param.bParam = ZGetSetpoint(); break;
        case ZUNO_CHANNEL1_SETTER: ZSetSetpoint(callback_data.param.bParam); break;

        case ZUNO_CHANNEL2_GETTER: callback_data.param.bParam = ZGetExteriorTemperature(); break;
        case ZUNO_CHANNEL2_SETTER: ZSetExteriorTemperature(callback_data.param.bParam); break;

        case ZUNO_CHANNEL3_GETTER: callback_data.param.wParam = ZGetRealTemperature(); break;

        case ZUNO_CHANNEL4_GETTER: callback_data.param.wParam = ZGetRealHumidity(); break;
        default: break;
    }
}
