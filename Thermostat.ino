/**
* @brief Thermostat on Z-uino
*
*/

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
unsigned long lastZwaveRefresh;
unsigned long const zaveRefreshDelay = 30000;
bool settingsError = false;

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

    lastZwaveRefresh = 0;
    //AUTOPID.ApplySettings();
    //AUTOPID.SetAutoTune(1);

    oled.begin();
    oled.clrscr();
}

/**
* @brief Main loop function
*
*/
void loop() {
    //if (!settingsError) {
    THERM.Loop();
    bool drawDisplay = false;
    if (THERM.GetMode() != lastMode || lastZwaveRefresh == 0) {
        MY_SERIAL.println("SP");
        lastMode = THERM.GetMode();
        zunoSendReport(1); // report setpoint
        drawDisplay = true;
    }

    if ((millis() > (lastZwaveRefresh + zaveRefreshDelay)) || (lastZwaveRefresh == 0)) {
        lastZwaveRefresh = millis();
        MY_SERIAL.println("ZR");
        if (SENSOR.GetTemperature() != lastTemp || lastZwaveRefresh == 0) {
            MY_SERIAL.println("T");
            lastTemp = SENSOR.GetTemperature();
            zunoSendReport(2); // report temperature
        }
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

    //}
    //else {
    //    MY_SERIAL.println("Stg er");
    //}

    delay(1000);
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
