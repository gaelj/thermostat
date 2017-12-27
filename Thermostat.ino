/**
* @brief Intelligent thermostat implementation on Z-uno platform
*
*/

#include "zwave_communication.h"
#include "zwave_communication.h"
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
#include "ThermostatRemoteConfig.h"
#include "zwave_communication.h"

#define MY_SERIAL Serial

// Custom sensor to retrieve temperature as a word (2 bytes)
#define ZUNO_SENSOR_MULTILEVEL_TEMPERATURE_WORD(GETTER) ZUNO_SENSOR_MULTILEVEL (ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE, SENSOR_MULTILEVEL_SCALE_CELSIUS, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_TWO_DECIMALS, GETTER)
#define ZUNO_SENSOR_MULTILEVEL_HUMIDITY_WORD(GETTER)    ZUNO_SENSOR_MULTILEVEL (ZUNO_SENSOR_MULTILEVEL_TYPE_RELATIVE_HUMIDITY, SENSOR_MULTILEVEL_SCALE_PERCENTAGE_VALUE, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_TWO_DECIMALS, GETTER)

ZUNO_SETUP_DEBUG_MODE(DEBUG_ON);


// Zwave channels: 1 channel to get/set the command id,
//                 1 channel to (get/)set the command value,
//                 1 channel to get the real temperature,
//                 1 channel to get the real humidity
ZUNO_SETUP_CHANNELS(ZUNO_SWITCH_MULTILEVEL(ZGetZtxCommand, ZSetZtxCommand)
    , ZUNO_SWITCH_MULTILEVEL(ZGetZtxValue, ZSetZtxValue)
    , ZUNO_SWITCH_MULTILEVEL(ZGetZrxValue, ZSetZrxValue)
    , ZUNO_SWITCH_MULTILEVEL(ZGetZrxValue, ZSetZrxValue)
    , ZUNO_SENSOR_MULTILEVEL_TEMPERATURE_WORD(ZGetRealTemperature)
    , ZUNO_SENSOR_MULTILEVEL_HUMIDITY_WORD(ZGetRealHumidity)
);

// Zwave associations: 1 group to set boiler relay on/off
// (association to the relay must be setup in the Zwave network master)
ZUNO_SETUP_ASSOCIATIONS(ZUNO_ASSOCIATION_GROUP_SET_VALUE);


byte ztxCommand;
byte ztxValue;
byte zrxCommand;
byte zrxValue;

// Create objects
static TimerClass BOILER_ON_TIMER(0);
static TimerClass PID_TIMER(BOILER_MIN_TIME);
static TimerClass ZWAVE_TIMER1(ZWAVE_PERIOD);
static TimerClass ZWAVE_TIMER2(ZWAVE_PERIOD);
static TimerClass ZWAVE_TIMER3(ZWAVE_PERIOD);
static TimerClass ZWAVE_TIMER4(ZWAVE_PERIOD);
static TimerClass ZWAVE_TIMER5(ZWAVE_PERIOD);
static TimerClass ZWAVE_TIMER6(ZWAVE_PERIOD);
static TimerClass ZWAVE_TIMER7(ZWAVE_PERIOD);
static TimerClass SENSOR_TIMER(OLED_SENSOR_PERIOD);
static SettingsClass SETTINGS;
static SensorClass SENSOR;
static BoilerClass BOILER;
static RemoteConfiguratorClass REMOTE;
static ZwaveCommunicationClass ZWAVE(&ztxCommand, &ztxValue);

static PID PIDREG(&SETTINGS);
/*
static PID_ATune atune;
static AutoPidClass AUTOPID(&pid, &atune, &SETTINGS, &MODE);
static ThermostatClass THERM(&SETTINGS, &SENSOR, &BOILER, &HIST, &MODE);
*/
static ThermostatClass THERM(&PIDREG, &SETTINGS, &SENSOR, &BOILER, &BOILER_ON_TIMER, &PID_TIMER, &REMOTE, &ZWAVE);
static LedControlClass LEDS(&SENSOR, &BOILER, &THERM, &REMOTE);
static OledDisplayClass DISPLAY(&SETTINGS, &SENSOR, &BOILER, &THERM, &PIDREG, &LEDS, &REMOTE);
static ButtonControlClass BUTTONS(&THERM, &LEDS, &DISPLAY, &REMOTE, &ZWAVE);

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
    ZWAVE_TIMER1.Start(0);
    ZWAVE_TIMER2.Start(2500);
    ZWAVE_TIMER3.Start(5000);
    ZWAVE_TIMER4.Start(7500);
    ZWAVE_TIMER5.Start(10000);
    ZWAVE_TIMER6.Start(12500);
    ZWAVE_TIMER7.Start(15000);
}

/**
* @brief Main loop function
*
*/
void loop()
{
    unsigned long loopStart = millis();

    if (SENSOR_TIMER.IsElapsedRestart())
        SENSOR.ReadSensor();

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

    // Update Zwave values
    if (ZWAVE_TIMER1.IsElapsedRestart())
        ZWAVE.SendCommandValue(Get_Mode, 0); // get the mode set in Domoticz

    if (ZWAVE_TIMER2.IsElapsedRestart())
        ZWAVE.SendCommandValue(Get_ExteriorTemperature1, 0);
    if (ZWAVE_TIMER3.IsElapsedRestart())
        ZWAVE.SendCommandValue(Get_ExteriorTemperature2, 0);

    if (ZWAVE_TIMER4.IsElapsedRestart())
        ZWAVE.SendCommandValue(Get_ExteriorHumidity1, 0);
    if (ZWAVE_TIMER5.IsElapsedRestart())
        ZWAVE.SendCommandValue(Get_ExteriorHumidity2, 0);

    if (ZWAVE_TIMER6.IsElapsedRestart())
        ZWAVE.ReportTemperature();

    if (ZWAVE_TIMER7.IsElapsedRestart())
        ZWAVE.ReportHumidity();

    // Run the thermostat loop
    THERM.Loop();

    // Wait if needed
    unsigned long loopTime = millis() - loopStart;
    delay(LOOP_DELAY > loopTime ? LOOP_DELAY - loopTime : 1);
}

/**
* @brief The PC requests the Zuno TX command
*
*/
byte ZGetZtxCommand()
{
    //Serial.print("ZTXCommand ");
    //Serial.println(ztxCommand);
    //LEDS.SetFlash(ZUNO_CALLBACK_COLOR);
    return ztxCommand;
}

/**
* @brief The PC requests the Zuno TX value
*
*/
byte ZGetZtxValue()
{
    //Serial.print("ZTXValue ");
    //Serial.println(ztxValue);
    //LEDS.SetFlash(ZUNO_CALLBACK_COLOR);
    return ztxValue;
}

/**
* @brief The Zuno gets a Zuno TX command from the PC (shouldn't happen)
*
*/
void ZSetZtxCommand(byte value)
{
    ztxCommand = value;
}

/**
* @brief The Zuno gets a Zuno TX value from the PC (shouldn't happen)
*
*/
void ZSetZtxValue(byte value)
{
    ztxValue = value;
}







/**
* @brief The PC requests the Zuno RX command (shouldn't happen)
*
*/
byte ZGetZrxCommand()
{
    //LEDS.SetFlash(ZUNO_CALLBACK_COLOR);
    return zrxCommand;
}

/**
* @brief The PC requests the Zuno RX value (shouldn't happen)
*
*/
byte ZGetZrxValue()
{
    //LEDS.SetFlash(GET_SETPOINT_COLOR);
    return zrxValue;
}

/**
* @brief The Zuno gets a Zuno RX command from the PC
*
*/
void ZSetZrxCommand(byte value)
{
    //Serial.print("ZRXCommand ");
    //Serial.println(value);
    REMOTE.SetCommand(Commands(value));
    zrxCommand = value;
}

/**
* @brief The Zuno gets a Zuno RX value from the PC
*
*/
void ZSetZrxValue(byte value)
{
    //Serial.print("ZRXValue ");
    //Serial.println(value);
    //LEDS.SetFlash(SET_SETPOINT_COLOR);
    REMOTE.SetValue(value);
    zrxValue = value;
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
word ZGetRealHumidity()
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
        case ZUNO_CHANNEL1_GETTER: callback_data.param.bParam = ZGetZtxCommand(); break;
        case ZUNO_CHANNEL1_SETTER: ZSetZtxCommand(callback_data.param.bParam); break;

        case ZUNO_CHANNEL2_GETTER: callback_data.param.bParam = ZGetZtxValue(); break;
        case ZUNO_CHANNEL2_SETTER: ZSetZtxValue(callback_data.param.bParam); break;

        case ZUNO_CHANNEL3_GETTER: callback_data.param.bParam = ZGetZrxCommand(); break;
        case ZUNO_CHANNEL3_SETTER: ZSetZrxCommand(callback_data.param.bParam); break;

        case ZUNO_CHANNEL4_GETTER: callback_data.param.bParam = ZGetZrxValue(); break;
        case ZUNO_CHANNEL4_SETTER: ZSetZrxValue(callback_data.param.bParam); break;
        /*
        case ZUNO_CHANNEL5_GETTER: callback_data.param.wParam = ZGetRealTemperature(); break;
        case ZUNO_CHANNEL6_GETTER: callback_data.param.wParam = ZGetRealHumidity(); break;
        */
        default: break;
    }
}
