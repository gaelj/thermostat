/**
* @brief Thermostat on Z-uino
*
*/

#include "pinout.h"
#include "sensor.h"

#include <EEPROM.h>
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
#include "icons.h"
#include "led.h"
#include "led_control.h"
#include "timer.h"
#include "button.h"

#define MY_SERIAL Serial

#define ZUNO_SENSOR_MULTILEVEL_TEMPERATURE_WORD(GETTER)    ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE, SENSOR_MULTILEVEL_SCALE_CELSIUS, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_TWO_DECIMALS, GETTER)	

ZUNO_SETUP_DEBUG_MODE(DEBUG_ON);

// Setup channels - we have 1 channel to get/set the desired temperature,
//                          1 channel to get the real temperature,
//                          1 channel to get the real humidity
ZUNO_SETUP_CHANNELS(ZUNO_SWITCH_MULTILEVEL(ZGetSetpoint, ZSetSetpoint)
    , ZUNO_SENSOR_MULTILEVEL_TEMPERATURE_WORD(ZGetRealTemperature)
    //,ZUNO_SENSOR_MULTILEVEL_HUMIDITY(RealHumidityGetter)
);

// Setup associations - we have 1 group for boiler on/off behavior
ZUNO_SETUP_ASSOCIATIONS(ZUNO_ASSOCIATION_GROUP_SET_VALUE);

ButtonClass BUTTON1(PIN_BUTTON1);
ButtonClass BUTTON2(PIN_BUTTON2);

TimerClass LED_BLINK_TIMER(1500);
TimerClass LED_FLASH_TIMER(100);
TimerClass LED_ANIMATION_TIMER(250);
TimerClass SENSOR_TIMER(10000);
TimerClass ZWAVE_TIMER(30000);
TimerClass TEMP_CHANGE_TIMER(30000);

OLED DISPLAY;
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

byte lastBoilerState = 1;
int lastTemp = 0;
ThermostatMode lastMode = Absent;
unsigned long const zaveRefreshDelay = 30000;

/**
* @brief Draw the screen
*
*/
void DrawDisplay() {

    // clear screen
    DISPLAY.clrscr();
    delay(5);

    // sensor temperature
    DISPLAY.setFont(zuno_font_numbers16);
    DISPLAY.gotoXY(13, 0);
    DISPLAY.fixPrint((long)(SENSOR.GetTemperature() * 10), 1);

    // setpoint temperature
    DISPLAY.gotoXY(13, 4);
    DISPLAY.fixPrint((long)(SETTINGS.GetSetPoint(MODE.CurrentThermostatMode) * 10), 1);

    // boiler state
    if (BOILER.GetBoilerState()) {
        DISPLAY.gotoXY(80, 4);
        DISPLAY.writeData(flame_data);
    }

    // thermostat mode
    DISPLAY.gotoXY(80, 0);
    switch (THERM.GetMode()) {
        case Day: DISPLAY.writeData(sun_data); break;
        case Night: DISPLAY.writeData(moon_data); break;
        case Absent: DISPLAY.writeData(absent_data); break;
        case Frost: DISPLAY.writeData(snow_data); break;
        case Warm: DISPLAY.writeData(hot_data); break;
    }
}

/**
* @brief Main setup function
*
*/
void setup() {
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

    DISPLAY.begin();
    DISPLAY.clrscr();

    SENSOR.ReadSensor();
    delay(100);
}

/**
* @brief Main loop function
*
*/
void loop() {
    // run the thermostat loop
    if (ZWAVE_TIMER.IsElapsed()) {
        ZWAVE_TIMER.Init();
        LEDS.FlashAll(COLOR_WHITE);
        THERM.Loop();
        //zunoSendReport(1); // report setpoint
        zunoSendReport(2); // report temperature
    }

    // handle on button pressed event
    int change = 0;
    if (BUTTON1.ButtonHasBeenPressed())
        change = 1;
    if (BUTTON2.ButtonHasBeenPressed())
        change = -1;
    if (change != 0) {
        int newMode = ((int)THERM.GetMode() + change) % THERMOSTAT_MODE_COUNT;
        if (newMode < 0) newMode = THERMOSTAT_MODE_COUNT - 1;
        THERM.SetMode(ThermostatMode(newMode));
        zunoSendReport(1); // report setpoint
    }
    
    // handle is button pressed state
    // if (BUTTON1.ButtonState == LOW || BUTTON2.ButtonState == LOW)
    //     LEDS.FlashAll(COLOR_WHITE);

    // boiler state changed
    LEDS.SetBlinkingState(BOILER.GetBoilerState());

    // Refresh display
    bool drawDisplay = false;
    if (THERM.GetMode() != lastMode) {
        lastMode = THERM.GetMode();
        drawDisplay = true;
    }

    if (SENSOR_TIMER.IsElapsed()) {
        SENSOR_TIMER.Init();
        SENSOR.ReadSensor();
        if (SENSOR.GetTemperature() != SENSOR.GetPreviousTemperature()) {
            drawDisplay = true;
        }
    }

    if (TEMP_CHANGE_TIMER.IsElapsed()) {
        TEMP_CHANGE_TIMER.Init();
        int delta = SENSOR.GetTemperature() - lastTemp;
        if (delta != 0) {
            int period = 100 / delta;
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

    if (BOILER.GetBoilerState() != lastBoilerState) {
        lastBoilerState = BOILER.GetBoilerState();
        drawDisplay = true;
    }

    if (drawDisplay)
        DrawDisplay();

    LEDS.DrawAll(THERM.GetMode());

    delay(10);
}


/**
* @brief Zwave Setter for Desired Temperature
*
*/
void ZSetSetpoint(byte value) {
    //MY_SERIAL.print("ZSetSetpoint to ");
    //MY_SERIAL.println(value);
    if (THERM.GetMode() != MODE.Decode(value)) {
        /*
        MY_SERIAL.print("Sp=");
        switch (MODE.Decode(value)) {
            case Frost: MY_SERIAL.println("Frost"); break;
            case Absent: MY_SERIAL.println("Absent"); break;
            case Night: MY_SERIAL.println("Night"); break;
            case Day: MY_SERIAL.println("Day"); break;
            case Warm: MY_SERIAL.println("Warm"); break;
        }
        */
        THERM.SetMode(MODE.Decode(value));
        zunoSendReport(1); // report setpoint
    }
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
}*/

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
    switch (callback_data.type) {
        case ZUNO_CHANNEL1_GETTER: callback_data.param.bParam = ZGetSetpoint(); break;
        case ZUNO_CHANNEL1_SETTER: ZSetSetpoint(callback_data.param.bParam); break;
        case ZUNO_CHANNEL2_GETTER: callback_data.param.wParam = ZGetRealTemperature(); break;
        default: break;
    }
}
