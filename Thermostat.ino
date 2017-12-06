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
#include "timer.h"

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


OLED oled;
LedClass LED1(PIN_LED_R1, PIN_LED_G1, PIN_LED_B1);
LedClass LED2(PIN_LED_R2, PIN_LED_G2, PIN_LED_B2);
LedClass LED3(PIN_LED_R3, PIN_LED_G3, PIN_LED_B3);
TimerClass LED_BLINK_TIMER(500);
TimerClass LED_FLASH_TIMER(100);
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

float lastTemp = 200;
float lastDrawnTemp = 200;
byte lastBoilerState = 1;
ThermostatMode lastMode = Absent;
unsigned long const zaveRefreshDelay = 30000;
int buttonState = HIGH;             // the current reading from the input pin
bool ledBlinkState = false;
byte ledColor = COLOR_BLACK;

/**
* @brief Draw the screen
*
*/
void DrawDisplay() {

    // clear screen
    oled.clrscr();
    delay(5);

    // sensor temperature
    oled.setFont(zuno_font_numbers16);
    oled.gotoXY(13, 0);
    oled.fixPrint((long)(SENSOR.GetTemperature() * 10), 1);

    // setpoint temperature
    oled.gotoXY(13, 4);
    oled.fixPrint((long)(SETTINGS.GetSetPoint(MODE.CurrentThermostatMode) * 10), 1);

    // boiler state
    if (BOILER.GetBoilerState()) {
        oled.gotoXY(80, 4);
        oled.writeData(flame_data);
    }

    // thermostat mode
    oled.gotoXY(80, 0);
    switch (THERM.GetMode()) {
        case Day: oled.writeData(sun_data); break;
        case Night: oled.writeData(moon_data); break;
        case Absent: oled.writeData(absent_data); break;
        case Frost: oled.writeData(snow_data); break;
        case Warm: oled.writeData(hot_data); break;
    }
}

bool ButtonHasBeenPressed() {
    int reading = digitalRead(PIN_BUTTON);
    if (reading != buttonState) {
        buttonState = reading;
        if (buttonState == LOW) {
            return true;
        }
    }
    return false;
}

/**
* @brief Main setup function
*
*/
void setup() {
    pinMode(PIN_BUTTON, INPUT);
    LED1.Begin();
    LED2.Begin();
    LED3.Begin();

    MY_SERIAL.begin(115200);
    //if (!SETTINGS.RestoreSettings()) {
    SETTINGS.LoadDefaults();
    //if (!SETTINGS.PersistSettings())
    //    settingsError = true;
    //}

    //AUTOPID.ApplySettings();
    //AUTOPID.SetAutoTune(1);

    oled.begin();
    oled.clrscr();

    ZWAVE_TIMER.Init();
    LED_BLINK_TIMER.IsActive = false;
    LED_FLASH_TIMER.IsActive = false;
}

/**
* @brief Main loop function
*
*/
void loop() {
    // run the thermostat loop
    if (ZWAVE_TIMER.IsElapsed()) {
        ZWAVE_TIMER.Init();
        LED1.DisplayColor(COLOR_WHITE);
        THERM.Loop();
        //zunoSendReport(1); // report setpoint
        zunoSendReport(2); // report temperature
    }

    // handle on button pressed event
    if (ButtonHasBeenPressed()) {
        ThermostatMode newMode;
        switch (THERM.GetMode()) {
            case Frost: newMode = Absent; break;
            case Absent: newMode = Night; break;
            case Night: newMode = Day; break;
            case Day: newMode = Warm; break;
            case Warm: newMode = Frost; break;
        }
        THERM.SetMode(newMode);
        //LED_FLASH_TIMER.Init();
        zunoSendReport(1); // report setpoint
    }
    
    // handle is button pressed state
    if (buttonState == LOW)
        LED_FLASH_TIMER.Init();

    // boiler state changed
    if (LED_BLINK_TIMER.IsActive != BOILER.GetBoilerState()) {
        LED_BLINK_TIMER.IsActive = BOILER.GetBoilerState();
        if (LED_BLINK_TIMER.IsActive)
            LED_BLINK_TIMER.Init();
    }

    // toggle blink
    if (LED_BLINK_TIMER.IsActive && LED_BLINK_TIMER.IsElapsed()) {
        ledBlinkState = !ledBlinkState;
        LED_BLINK_TIMER.Init();
    }

    // Set LED color
    ledColor = COLOR_BLACK;
    if (LED_FLASH_TIMER.IsActive && !LED_FLASH_TIMER.IsElapsed())
        ledColor = COLOR_WHITE;
    else if (!LED_BLINK_TIMER.IsActive || !ledBlinkState) {
        switch (THERM.GetMode()) {
            case Absent: ledColor = COLOR_YELLOW; break;
            case Night: ledColor = COLOR_MAGENTA; break;
            case Day: ledColor = COLOR_GREEN; break;
            case Warm: ledColor = COLOR_RED; break;
            case Frost: ledColor = COLOR_BLUE; break;
        }
    }
    LED1.DisplayColor(ledColor);

    // Refresh display
    bool drawDisplay = false;
    if (THERM.GetMode() != lastMode) {
        lastMode = THERM.GetMode();
        drawDisplay = true;
    }

    if (SENSOR.GetTemperature() != lastDrawnTemp) {
        lastDrawnTemp = SENSOR.GetTemperature();
        drawDisplay = true;
    }

    if (BOILER.GetBoilerState() != lastBoilerState) {
        lastBoilerState = BOILER.GetBoilerState();
        drawDisplay = true;
    }

    if (drawDisplay)
        DrawDisplay();

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
        Serial.print("Sp=");
        switch (MODE.Decode(value)) {
            case Frost: Serial.println("Frost"); break;
            case Absent: Serial.println("Absent"); break;
            case Night: Serial.println("Night"); break;
            case Day: Serial.println("Day"); break;
            case Warm: Serial.println("Warm"); break;
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
