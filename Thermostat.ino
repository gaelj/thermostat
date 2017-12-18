/**
* @brief Thermostat on Z-uino
*
*/

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
#include "oleddisplay.h"

#define MY_SERIAL Serial

#define ZUNO_SENSOR_MULTILEVEL_TEMPERATURE_WORD(GETTER)    ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE, SENSOR_MULTILEVEL_SCALE_CELSIUS, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_TWO_DECIMALS, GETTER)	

ZUNO_SETUP_DEBUG_MODE(DEBUG_ON);

#define ZUNO_REPORT_SETPOINT    1
#define ZUNO_REPORT_TEMP        2

// Zwave channels: 1 channel to get/set the desired temperature,
//                 1 channel to get the real temperature,
//                 1 channel to get the real humidity
ZUNO_SETUP_CHANNELS(ZUNO_SWITCH_MULTILEVEL(ZGetSetpoint, ZSetSetpoint)
    , ZUNO_SENSOR_MULTILEVEL_TEMPERATURE_WORD(ZGetRealTemperature)
    //,ZUNO_SENSOR_MULTILEVEL_HUMIDITY(RealHumidityGetter)
);

// Zwave associations: 1 group to set boiler relay on/off
// (association to the relay must be setup in the Zwave network master)
ZUNO_SETUP_ASSOCIATIONS(ZUNO_ASSOCIATION_GROUP_SET_VALUE);

// Create objects
ButtonClass BUTTON1(PIN_BUTTON1);
ButtonClass BUTTON2(PIN_BUTTON2);

TimerClass LED_BLINK_TIMER(1500);
TimerClass LED_FLASH_TIMER(100);
TimerClass LED_ANIMATION_TIMER(250);
TimerClass SENSOR_TIMER(10000);
TimerClass ZWAVE_TIMER(30000);
TimerClass TEMP_CHANGE_TIMER(30000);

settings_s TheSettings;
SettingsClass SETTINGS(&TheSettings);
ThermostatModeClass MODE;
HysteresisClass HIST(&TheSettings);
SensorClass SENSOR;
BoilerClass BOILER;

LedClass LED0(PIN_LED_R1, PIN_LED_G1, PIN_LED_B1);
LedClass LED1(PIN_LED_R2, PIN_LED_G2, PIN_LED_B2);
LedClass LED2(PIN_LED_R3, PIN_LED_G3, PIN_LED_B3);
LedControlClass LEDS(&LED0, &LED1, &LED2, &LED_FLASH_TIMER, &LED_BLINK_TIMER, &LED_ANIMATION_TIMER);
/*
PID pid(&SETTINGS);
PID_ATune atune;
AutoPidClass AUTOPID(&pid, &atune, &SETTINGS, &MODE);
ThermostatClass THERM(&AUTOPID, &SETTINGS, &SENSOR, &BOILER, &HIST, &MODE);
*/
ThermostatClass THERM(&SETTINGS, &SENSOR, &BOILER, &HIST, &MODE);
DisplayClass DISPLAY(&SETTINGS, &SENSOR, &BOILER, &THERM, &MODE);

byte lastBoilerState = 1;
int lastTemp = 0;
ThermostatMode lastMode = Absent;

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

    BUTTON1.Init();
    BUTTON2.Init();
    LEDS.Init();
    DISPLAY.Init();
    SENSOR.ReadSensor();
    lastTemp = SENSOR.GetTemperature();
}

/**
* @brief Main loop function
*
*/
void loop()
{
    // Run the thermostat loop
    if (ZWAVE_TIMER.IsElapsed()) {
        ZWAVE_TIMER.Init();
        THERM.Loop();
        //zunoSendReport(ZUNO_REPORT_SETPOINT);
        zunoSendReport(ZUNO_REPORT_TEMP);
    }

    // Handle button presses
    int change = 0;
    if (BUTTON1.ButtonHasBeenPressed())
        change = 1;
    if (BUTTON2.ButtonHasBeenPressed())
        change = -1;
    if (change != 0) {
        int newMode = ((int)THERM.GetMode() + change) % THERMOSTAT_MODE_COUNT;
        if (newMode < 0) newMode = THERMOSTAT_MODE_COUNT - 1;
        THERM.SetMode(ThermostatMode(newMode));
        zunoSendReport(ZUNO_REPORT_SETPOINT);
    }

    // Refresh display if thermostat mode has changed
    bool drawDisplay = false;
    if (THERM.GetMode() != lastMode) {
        lastMode = THERM.GetMode();
        drawDisplay = true;
    }

    // Refresh display if temperature has changed
    if (SENSOR_TIMER.IsElapsed()) {
        SENSOR_TIMER.Init();
        SENSOR.ReadSensor();
        if (SENSOR.GetTemperature() != SENSOR.GetPreviousTemperature()) {
            drawDisplay = true;
        }
    }

    // Refresh display if boiler state has changed
    if (BOILER.GetBoilerState() != lastBoilerState) {
        lastBoilerState = BOILER.GetBoilerState();
        drawDisplay = true;
    }

    // Refresh display if required
    if (drawDisplay)
        DISPLAY.DrawDisplay();

    // Flash LEDs on button pressed
    // if (BUTTON1.ButtonState == LOW || BUTTON2.ButtonState == LOW)
    //     LEDS.FlashAll(COLOR_WHITE);

    // Set LED blinking if boiler is on
    LEDS.SetBlinkingState(BOILER.GetBoilerState());

    // Set LED animation if temperature has changed
    if (TEMP_CHANGE_TIMER.IsElapsed()) {
        TEMP_CHANGE_TIMER.Init();
        float delta = SENSOR.GetTemperature() - lastTemp;
        if (delta != 0) {
            int period = 100 / abs(delta);
            if (period < 100) period = 100;
            if (period > 1000) period = 1000;
            if (delta > 0)
                LEDS.SetAnimation(1, period);
            else if (delta < 0)
                LEDS.SetAnimation(-1, period);
        }
        else
            LEDS.SetAnimation(0, 250);
        lastTemp = SENSOR.GetTemperature();
    }

    // Refresh LED states
    LEDS.DrawAll(THERM.GetMode());

    delay(1);
}


/**
* @brief Zwave Setter for Desired Temperature
*
*/
void ZSetSetpoint(byte value)
{
    if (THERM.GetMode() != MODE.Decode(value)) {
        LEDS.FlashAll(COLOR_BLUE);
        THERM.SetMode(MODE.Decode(value));
        zunoSendReport(ZUNO_REPORT_SETPOINT);
    }
}

/**
* @brief Zwave Getter for Desired Temperature
*
*/
byte ZGetSetpoint()
{
    LEDS.FlashAll(COLOR_YELLOW);
    return MODE.Encode(THERM.GetMode());
}

/**
* @brief Zwave Getter for Real Temperature
*
*/
word ZGetRealTemperature()
{
    LEDS.FlashAll(COLOR_GREEN);
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
    LEDS.FlashAll(COLOR_WHITE);
    switch (callback_data.type) {
        case ZUNO_CHANNEL1_GETTER: callback_data.param.bParam = ZGetSetpoint(); break;
        case ZUNO_CHANNEL1_SETTER: ZSetSetpoint(callback_data.param.bParam); break;
        case ZUNO_CHANNEL2_GETTER: callback_data.param.wParam = ZGetRealTemperature(); break;
        default: break;
    }
}
