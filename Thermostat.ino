/**
* @brief Intelligent thermostat implementation on Z-uno platform
*
*/

#include "button_control.h"
#include "pinout.h"
#include "sensor.h"

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

//#include <PID_v1.h>
//#include <PID_AutoTune_v0.h>

#include "settings.h"
#include "boiler.h"
#include "hysteresis.h"
#include "thermo_control.h"
#include "thermostat_mode.h"
#include "led.h"
#include "led_control.h"
#include "timer.h"
#include "button.h"
#include "button_control.h"
#include "oleddisplay.h"

#define MY_SERIAL Serial

#define ZUNO_SENSOR_MULTILEVEL_TEMPERATURE_WORD(GETTER)   ZUNO_SENSOR_MULTILEVEL (ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE, SENSOR_MULTILEVEL_SCALE_CELSIUS, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_TWO_DECIMALS, GETTER)

ZUNO_SETUP_DEBUG_MODE(DEBUG_ON);

#define ZUNO_REPORT_SETPOINT    1
#define ZUNO_REPORT_EXT_TEMP    2
#define ZUNO_REPORT_TEMP        3

// Zwave channels: 1 channel to get/set the desired temperature,
//                 1 channel to (get/)set the exterior temperature,
//                 1 channel to get the real temperature,
//                 1 channel to get the real humidity
ZUNO_SETUP_CHANNELS(ZUNO_SWITCH_MULTILEVEL(ZGetSetpoint, ZSetSetpoint)
    , ZUNO_SWITCH_MULTILEVEL(ZGetExteriorTemperature, ZSetExteriorTemperature)
    , ZUNO_SENSOR_MULTILEVEL_TEMPERATURE_WORD(ZGetRealTemperature)
    //, ZUNO_SENSOR_MULTILEVEL_HUMIDITY(RealHumidityGetter)
);

// Zwave associations: 1 group to set boiler relay on/off
// (association to the relay must be setup in the Zwave network master)
ZUNO_SETUP_ASSOCIATIONS(ZUNO_ASSOCIATION_GROUP_SET_VALUE);

// Create objects
TimerClass ZWAVE_TIMER(30000);
settings_s TheSettings;
SettingsClass SETTINGS(&TheSettings);
ThermostatModeClass MODE;
HysteresisClass HIST(&TheSettings);
SensorClass SENSOR;
BoilerClass BOILER;
/*
PID pid(&SETTINGS);
PID_ATune atune;
AutoPidClass AUTOPID(&pid, &atune, &SETTINGS, &MODE);
ThermostatClass THERM(&AUTOPID, &SETTINGS, &SENSOR, &BOILER, &HIST, &MODE);
*/
ThermostatClass THERM(&SETTINGS, &SENSOR, &BOILER, &HIST, &MODE);
OledDisplayClass DISPLAY(&SETTINGS, &SENSOR, &BOILER, &THERM);
LedControlClass LEDS(&SENSOR, &BOILER, &THERM);
ButtonControlClass BUTTONS(&THERM, &LEDS, &DISPLAY);


/**
* @brief Main setup function
*
*/
void setup()
{
    MY_SERIAL.begin(115200);
    //if (!SETTINGS.RestoreSettings()) {
    SETTINGS.LoadDefaults();
    //if (!SETTINGS.PersistSettings())
    //    settingsError = true;
    //}

    //AUTOPID.ApplySettings();
    //AUTOPID.SetAutoTune(1);

    SENSOR.ReadSensor();

    BUTTONS.Init();
    LEDS.Init();
    DISPLAY.Init();
}

/**
* @brief Main loop function
*
*/
void loop()
{
    unsigned long loopStart = millis();

    // Handle button presses
    BUTTONS.HandlePressedButtons();

    // Refresh display if required
    if (DISPLAY.DisplayRedrawNeeded())
        DISPLAY.DrawDisplay();

    // Set LED blinking if boiler is on
    LEDS.SetBlinkingState();

    // Set LED animation if temperature has changed
    LEDS.SetAnimationState();

    // Refresh LED states
    LEDS.DrawAll();

    // Run the thermostat loop
    if (ZWAVE_TIMER.IsElapsed()) {
        ZWAVE_TIMER.Start();
        THERM.Loop();
        zunoSendReport(ZUNO_REPORT_SETPOINT);
        zunoSendReport(ZUNO_REPORT_EXT_TEMP);
        zunoSendReport(ZUNO_REPORT_TEMP);
    }

    // Wait if needed
    const unsigned long loopDelay = 10;
    if (loopDelay > (millis() - loopStart))
        delay(loopDelay - (millis() - loopStart));
}


/**
* @brief Zwave Getter for Desired Temperature
*
*/
byte ZGetSetpoint()
{
    LEDS.SetFlash(GET_SETPOINT_COLOR);
    return MODE.Encode(THERM.GetMode());
}

/**
* @brief Zwave Setter for Desired Temperature
*
*/
void ZSetSetpoint(byte value)
{
    if (THERM.GetMode() != MODE.Decode(value)) {
        LEDS.SetFlash(SET_SETPOINT_COLOR);
        THERM.SetMode(MODE.Decode(value));
        //zunoSendReport(ZUNO_REPORT_SETPOINT);
    }
}

/**
* @brief Zwave Getter for external Temperature
*
*/
byte ZGetExteriorTemperature()
{
    LEDS.SetFlash(GET_SETPOINT_COLOR);
    return THERM.EncodeTemperature(THERM.ExteriorTemperature);
}

/**
* @brief Zwave Setter for external Temperature
*
*/
void ZSetExteriorTemperature(byte value)
{
    if (THERM.ExteriorTemperature != THERM.DecodeTemperature(value)) {
        LEDS.SetFlash(SET_SETPOINT_COLOR);
        THERM.ExteriorTemperature = THERM.DecodeTemperature(value);
        //zunoSendReport(ZUNO_REPORT_SETPOINT);
    }
}

/**
* @brief Zwave Getter for Real Temperature
*
*/
word ZGetRealTemperature()
{
    LEDS.SetFlash(GET_TEMPRATURE_COLOR);
    return SENSOR.Encode(SENSOR.GetTemperature());
}

/**
* @brief Zwave Getter for Real Humidity
*
*//*
byte RealHumidityGetter()
{
    return fromFloat(THERM.GetHumidity());
}*/

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
    LEDS.SetFlash(ZUNO_CALLBACK_COLOR);
    switch (callback_data.type) {
        case ZUNO_CHANNEL1_GETTER: callback_data.param.bParam = ZGetSetpoint(); break;
        case ZUNO_CHANNEL1_SETTER: ZSetSetpoint(callback_data.param.bParam); break;

        case ZUNO_CHANNEL2_GETTER: callback_data.param.bParam = ZGetExteriorTemperature(); break;
        case ZUNO_CHANNEL2_SETTER: ZSetExteriorTemperature(callback_data.param.bParam); break;

        case ZUNO_CHANNEL3_GETTER: callback_data.param.wParam = ZGetRealTemperature(); break;
        default: break;
    }
}
