/**
* @brief Intelligent thermostat implementation on Z-uno platform
*
*/

#include "globals.h"
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

// Custom sensor to retrieve temperature as a word (2 bytes)
#define ZUNO_SENSOR_MULTILEVEL_TEMPERATURE_WORD(GETTER) ZUNO_SENSOR_MULTILEVEL (ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE, SENSOR_MULTILEVEL_SCALE_CELSIUS, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_TWO_DECIMALS, GETTER)
#define ZUNO_SENSOR_MULTILEVEL_HUMIDITY_WORD(GETTER)    ZUNO_SENSOR_MULTILEVEL (ZUNO_SENSOR_MULTILEVEL_TYPE_RELATIVE_HUMIDITY, SENSOR_MULTILEVEL_SCALE_PERCENTAGE_VALUE, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_TWO_DECIMALS, GETTER)

// Zwave channels: 1 channel to get/set the command id,
//                 1 channel to (get/)set the command value,
//                 1 channel to get the real temperature,
//                 1 channel to get the real humidity
ZUNO_SETUP_CHANNELS(ZUNO_SENSOR_MULTILEVEL_TEMPERATURE_WORD(ZGetSensorTemperature),
                    ZUNO_SENSOR_MULTILEVEL_HUMIDITY_WORD(ZGetSensorHumidity),
                    ZUNO_SWITCH_BINARY(ZGetBoilerState, ZSetBoilerState),
                    ZUNO_SWITCH_MULTILEVEL(ZGetSetpoint, ZSetSetpoint),
                    ZUNO_SWITCH_MULTILEVEL(ZGetMode, ZSetMode),
                    ZUNO_SWITCH_MULTILEVEL(ZGetExteriorTemperature, ZSetExteriorTemperature),
                    ZUNO_SWITCH_MULTILEVEL(ZGetExteriorHumidity, ZSetExteriorHumidity),
                    ZUNO_SWITCH_MULTILEVEL(ZGetExteriorPressure, ZSetExteriorPressure)
);

// Zwave associations: 1 group to set boiler relay on/off
// (association to the relay must be setup in the Zwave network master)
ZUNO_SETUP_ASSOCIATIONS(ZUNO_ASSOCIATION_GROUP_SET_VALUE);


// Create objects
static TimerClass ZWAVE_REPORT_TIMER(ZWAVE_PERIOD);
static TimerClass SENSOR_TIMER(READ_SENSOR_PERIOD);
static TimerClass MODE_SET_DELAY_TIMER(MODE_SET_DELAY_PERIOD);
// static PID_ATune atune;
// static AutoPidClass AUTOPID(&pid, &atune, &SETTINGS, &MODE);

PID PIDREG;
params_t Prm;
zwave_values_t zwave_values;

ButtonActions buttonAction;
unsigned long loopStart;
unsigned long loopTime;

#define MY_SERIAL   Serial

/**
* @brief Main setup function
*
*/
void setup()
{
#ifdef LOGGING_ACTIVE
    MY_SERIAL.begin(115200);
#endif // LOGGING_ACTIVE

    Settings_LoadDefaults();
    OledDisplay_Init();
    Thermostat_Init();

    //if (!SETTINGS.RestoreSettings()) {
    //SETTINGS.LoadDefaults();
    //if (!SETTINGS.PersistSettings())
    //    settingsError = true;
    //}

    //AUTOPID.ApplySettings();
    //AUTOPID.SetAutoTune(1);

    Prm.CurrentThermostatMode = NoMode;
    Prm.ExteriorTemperature = 0;
    Prm.ExteriorHumidity = 0;
    Prm.IlluminationPower = true;

    zwave_values.Mode = 0;
}

/**
* @brief Main loop function
*
*/
void loop()
{
    loopStart = millis();

    // Read local temperature & humidity sensor
    if (SENSOR_TIMER.IsElapsedRestart())
        ReadSensor();

    // Handle button presses
    buttonAction = ReadButtons();
    if (buttonAction == Button12 || (!Prm.IlluminationPower && buttonAction != NoButtonAction)) {
        Prm.IlluminationPower = !Prm.IlluminationPower;
        OledDisplay_SetPower(Prm.IlluminationPower);
    }
    else if (buttonAction == Button1) {
        MODE_SET_DELAY_TIMER.Start();
        zwave_values.Mode = ((zwave_values.Mode + 1) % THERMOSTAT_MODE_COUNT);
        if (zwave_values.Mode == 0)
            zwave_values.Mode = 1;
        Thermostat_SetMode(ThermostatMode(zwave_values.Mode));
    }
    else if (buttonAction == Button2) {
        OledDisplay_ShowNextPage();
    }

    // Wait before reporting ZWave, to end only the final selection when "cycling" through modes
    if (MODE_SET_DELAY_TIMER.IsActive && MODE_SET_DELAY_TIMER.IsElapsed()) {
        zunoSendReport(ZUNO_REPORT_MODE);
    }

    // Set LED blinking if boiler is on
    LedsSetBlinkingState();

    // Set LED animation if temperature has changed
    LedsSetAnimationState();

    // Refresh LED states
    LedsDrawAll();

    // Refresh OLED display
    OledDisplay_DrawDisplay();

    // Report sensor values
    if (ZWAVE_REPORT_TIMER.IsElapsedRestart()) {
        zwave_values.SensorTemperature = EncodeSensorReading(SensorTemperature);
        zwave_values.SensorHumidity = EncodeSensorReading(SensorHumidity);
        zunoSendReport(ZUNO_REPORT_TEMP);
        zunoSendReport(ZUNO_REPORT_HUMIDITY);

        if (zwave_values.Mode == 0) {
#ifdef LOGGING_ACTIVE
            Serial.println("Req Mode 99");
#endif // LOGGING_ACTIVE
            zwave_values.Mode == 99;
            zunoSendReport(ZUNO_REPORT_MODE);
        }
        else if (zwave_values.Mode == 99) {
#ifdef LOGGING_ACTIVE
            Serial.println("Req Mode 0");
#endif // LOGGING_ACTIVE
            zwave_values.Mode == 0;
            zunoSendReport(ZUNO_REPORT_MODE);
        }
        if (zwave_values.ExteriorTemperature == 0) {
            zunoSendReport(ZUNO_REPORT_EXT_TEMP);
            zunoSendReport(ZUNO_REPORT_EXT_HUM);
            zunoSendReport(ZUNO_REPORT_EXT_PRESS);
        }
    }

    // Process any received Zwave values
    if (zwave_values.BoilerState != CurrentBoilerState) {
        SetBoilerState(zwave_values.BoilerState);
    }
    if (DecodeTemp(zwave_values.Setpoint) != TheSettings.CurrentSetPoint) {
        Thermostat_SetSetPoint(DecodeTemp(zwave_values.Setpoint));
    }
    if (zwave_values.Mode < 99 && ThermostatMode(zwave_values.Mode) != Prm.CurrentThermostatMode) {
        Thermostat_SetMode(ThermostatMode(zwave_values.Mode));
    }
    if (DecodeTemp(zwave_values.ExteriorTemperature) != Prm.ExteriorTemperature) {
        Prm.ExteriorTemperature = DecodeTemp(zwave_values.ExteriorTemperature);
    }
    if (DecodeHumidity(zwave_values.ExteriorHumidity) != Prm.ExteriorHumidity) {
        Prm.ExteriorHumidity = DecodeHumidity(zwave_values.ExteriorHumidity);
    }
    if (DecodePressure(zwave_values.ExteriorPressure) != Prm.ExteriorPressure) {
        Prm.ExteriorPressure = DecodePressure(zwave_values.ExteriorPressure);
    }

    // Run the thermostat loop
    if (currentPage != OLED_PAGE_COUNT)
        Thermostat_Loop();

    // Wait if needed
    loopTime = millis() - loopStart;
    delay(LOOP_DELAY > loopTime ? LOOP_DELAY - loopTime : 1);
}


byte ZGetBoilerState()
{
    return zwave_values.BoilerState;
}
void ZSetBoilerState(byte value)
{
    zwave_values.BoilerState = value;
}

byte ZGetSetpoint()
{
    return zwave_values.Setpoint;
}
void ZSetSetpoint(byte value)
{
    zwave_values.Setpoint = value;
}

byte ZGetMode()
{
    return zwave_values.Mode;
}
void ZSetMode(byte value)
{
    zwave_values.Mode = value;
}

byte ZGetExteriorTemperature()
{
    return zwave_values.ExteriorTemperature;
}
void ZSetExteriorTemperature(byte value)
{
    zwave_values.ExteriorTemperature = value;
}

byte ZGetExteriorHumidity()
{
    return zwave_values.ExteriorHumidity;
}
void ZSetExteriorHumidity(byte value)
{
    zwave_values.ExteriorHumidity = value;
}

byte ZGetExteriorPressure()
{
    return zwave_values.ExteriorPressure;
}
void ZSetExteriorPressure(byte value)
{
    zwave_values.ExteriorPressure = value;
}

/**
* @brief Zwave Getter for Real Temperature
*
*/
word ZGetSensorTemperature()
{
    return zwave_values.SensorTemperature;
}

/**
* @brief Zwave Getter for Real Humidity
*
*/
word ZGetSensorHumidity()
{
    return zwave_values.SensorHumidity;
}

/**
* @brief Universal handler for all the channels
*
* @remark See callback_data variable
*         We use zero based index of the channel instead of typical
*         Getter/Setter index of Z-Uno.
*         See enum ZUNO_CHANNEL*_GETTER/ZUNO_CHANNEL*_SETTER in ZUNO_Definitions.h
*/
void zunoCallback(void)
{
    switch (callback_data.type) {
        case ZUNO_CHANNEL1_GETTER: callback_data.param.wParam = ZGetSensorTemperature(); break;

        case ZUNO_CHANNEL2_GETTER: callback_data.param.wParam = ZGetSensorHumidity(); break;

        case ZUNO_CHANNEL3_GETTER: callback_data.param.bParam = ZGetBoilerState(); break;
        case ZUNO_CHANNEL3_SETTER: ZSetBoilerState(callback_data.param.bParam); break;

        case ZUNO_CHANNEL4_GETTER: callback_data.param.bParam = ZGetSetpoint(); break;
        case ZUNO_CHANNEL4_SETTER: ZSetSetpoint(callback_data.param.bParam); break;

        case ZUNO_CHANNEL5_GETTER: callback_data.param.bParam = ZGetMode(); break;
        case ZUNO_CHANNEL5_SETTER: ZSetMode(callback_data.param.bParam); break;

        case ZUNO_CHANNEL6_GETTER: callback_data.param.bParam = ZGetExteriorTemperature(); break;
        case ZUNO_CHANNEL6_SETTER: ZSetExteriorTemperature(callback_data.param.bParam); break;

        case ZUNO_CHANNEL7_GETTER: callback_data.param.bParam = ZGetExteriorHumidity(); break;
        case ZUNO_CHANNEL7_SETTER: ZSetExteriorHumidity(callback_data.param.bParam); break;

        case ZUNO_CHANNEL8_GETTER: callback_data.param.bParam = ZGetExteriorPressure(); break;
        case ZUNO_CHANNEL8_SETTER: ZSetExteriorPressure(callback_data.param.bParam); break;

        default: break;
    }
}
