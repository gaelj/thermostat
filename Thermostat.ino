/**
* @brief Intelligent thermostat implementation on Z-uno platform
*
*/

#include "radiator.h"
#include "globals.h"
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
#include "radiator.h"

#define MY_SERIAL Serial

// Custom sensor to retrieve temperature as a word (2 bytes)
#define ZUNO_SENSOR_MULTILEVEL_TEMPERATURE_WORD(GETTER) ZUNO_SENSOR_MULTILEVEL (ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE, SENSOR_MULTILEVEL_SCALE_CELSIUS, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_TWO_DECIMALS, GETTER)
#define ZUNO_SENSOR_MULTILEVEL_HUMIDITY_WORD(GETTER)    ZUNO_SENSOR_MULTILEVEL (ZUNO_SENSOR_MULTILEVEL_TYPE_RELATIVE_HUMIDITY, SENSOR_MULTILEVEL_SCALE_PERCENTAGE_VALUE, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_TWO_DECIMALS, GETTER)

ZUNO_SETUP_DEBUG_MODE(DEBUG_ON);

// Zwave channels: 1 channel to get/set the command id,
//                 1 channel to (get/)set the command value,
//                 1 channel to get the real temperature,
//                 1 channel to get the real humidity
ZUNO_SETUP_CHANNELS(
    ZUNO_SWITCH_MULTILEVEL(ZGetZtxCommand, ZSetZtxCommand)
    , ZUNO_SWITCH_MULTILEVEL(ZGetZtxValue, ZSetZtxValue)
    , ZUNO_SWITCH_MULTILEVEL(ZGetZrxValue, ZSetZrxValue)
    , ZUNO_SWITCH_MULTILEVEL(ZGetZrxValue, ZSetZrxValue)
    , ZUNO_SENSOR_MULTILEVEL_TEMPERATURE_WORD(ZGetRealTemperature)
    , ZUNO_SENSOR_MULTILEVEL_HUMIDITY_WORD(ZGetRealHumidity)
);

// Zwave associations: 1 group to set boiler relay on/off
// (association to the relay must be setup in the Zwave network master)
ZUNO_SETUP_ASSOCIATIONS(ZUNO_ASSOCIATION_GROUP_SET_VALUE);


// Create objects
static TimerClass ZWAVE_TIMER(ZWAVE_LONG_PERIOD);
static TimerClass ZWAVE_RX_TIMER(ZWAVE_SHORT_PERIOD - 500);
static TimerClass SENSOR_TIMER(READ_SENSOR_PERIOD);
static TimerClass MODE_SET_DELAY_TIMER(MODE_SET_DELAY_PERIOD);
static PID PIDREG;
// static PID_ATune atune;
// static AutoPidClass AUTOPID(&pid, &atune, &SETTINGS, &MODE);
static ThermostatClass THERM(&PIDREG);
static OledDisplayClass DISPLAY(&PIDREG);

radiator_s Radiators[6];
params_s Prm;

ButtonActions buttonAction;
byte zwaveMessageCounter = 0;
unsigned long loopStart;
unsigned long loopTime;
Commands TXCommand = No_Command;

#define ZWAVE_MSG_COUNT     17

/**
* @brief Main setup function
*
*/
void setup()
{
    MY_SERIAL.begin(115200);
    Settings_LoadDefaults();
    Remote_InitParameters();

    //if (!SETTINGS.RestoreSettings()) {
    //SETTINGS.LoadDefaults();
    //if (!SETTINGS.PersistSettings())
    //    settingsError = true;
    //}

    //AUTOPID.ApplySettings();
    //AUTOPID.SetAutoTune(1);
    ReportTXCommandValue(99, 99);
    ReportRXCommandValue(99, 99);
    delay(200);
}

/**
* @brief Main loop function
*
*/
void loop()
{
    loopStart = millis();

    // Process any received command / value ZWave input
    if (ZWAVE_RX_TIMER.IsActive && ZWAVE_RX_TIMER.IsElapsed()) {
        Serial.print("zrxCommand: ");
        Serial.println(zrxCommand);
        Serial.print("TXCommand: ");
        Serial.println(TXCommand);
        if (Commands(zrxCommand) == TXCommand) {
            Remote_SetCommand(Commands(zrxCommand));
            Remote_SetValue(zrxValue);
        }
        else {
            if (zwaveMessageCounter == 0)
                zwaveMessageCounter = ZWAVE_MSG_COUNT - 1;
            else
                zwaveMessageCounter--;
        }
    }

    if (SENSOR_TIMER.IsElapsedRestart())
        ReadSensor();

    // Handle button presses
    buttonAction = ReadButtons();
    if (buttonAction == Button12 || (!Prm.IlluminationPower && buttonAction != NoButtonAction)) {
        Prm.IlluminationPower = !Prm.IlluminationPower;
        //LEDS.SetPower(Prm.IlluminationPower);
        DISPLAY.SetPower(Prm.IlluminationPower);
    }
    else if (buttonAction == Button1) {
        MODE_SET_DELAY_TIMER.Start();
        ThermostatMode newMode = ThermostatMode((byte(Prm.CurrentThermostatMode) + 1) % THERMOSTAT_MODE_COUNT);
        Prm.CurrentThermostatMode = newMode;
    }
    else if (buttonAction == Button2) {
        DISPLAY.ShowNextPage();
    }

    // Wait before reporting ZWave, to end only the final selection when "cycling" through modes
    if (MODE_SET_DELAY_TIMER.IsActive && MODE_SET_DELAY_TIMER.IsElapsed()) {
        ReportTXCommandValue(Set_Mode, EncodeMode(Prm.CurrentThermostatMode));
    }

    // Set LED blinking if boiler is on
    LedsSetBlinkingState();

    // Set LED animation if temperature has changed
    LedsSetAnimationState();

    // Refresh LED states
    LedsDrawAll();

    // Refresh OLED display
    DISPLAY.DrawDisplay();

    // Update Zwave values
    if (ZWAVE_TIMER.IsElapsedRestart()) {
        MY_SERIAL.print("ZWave TX: cnt = ");
        MY_SERIAL.print(zwaveMessageCounter);

        switch (zwaveMessageCounter) {
            case 0: TXCommand = Get_Mode;
                    ReportTemperature();
                    ReportHumidity();
                    break;
            case 1: TXCommand = Get_ExteriorTemperature1; break;
            case 2: TXCommand = Get_ExteriorTemperature2; break;
            case 3: TXCommand = Get_ExteriorHumidity1; break;
            case 4: TXCommand = Get_ExteriorHumidity2; break;

            case 5: TXCommand = Get_Radiator0Setpoint1; break;
            case 6: TXCommand = Get_Radiator0Setpoint2; break;
            case 7: TXCommand = Get_Radiator0Temperature1; break;
            case 8: TXCommand = Get_Radiator0Temperature2; break;

            case 9: TXCommand = Get_Radiator1Setpoint1; break;
            case 10: TXCommand = Get_Radiator1Setpoint2; break;
            case 11: TXCommand = Get_Radiator1Temperature1; break;
            case 12: TXCommand = Get_Radiator1Temperature2; break;

            case 13: TXCommand = Get_Radiator2Setpoint1; break;
            case 14: TXCommand = Get_Radiator2Setpoint2; break;
            case 15: TXCommand = Get_Radiator2Temperature1; break;
            case 16: TXCommand = Get_Radiator2Temperature2; break;
            /*
            case 17: TXCommand = Get_Radiator3Setpoint1; break;
            case 18: TXCommand = Get_Radiator3Setpoint2; break;
            case 19: TXCommand = Get_Radiator3Temperature1; break;
            case 20: TXCommand = Get_Radiator3Temperature2; break;

            case 21: TXCommand = Get_Radiator4Setpoint1; break;
            case 22: TXCommand = Get_Radiator4Setpoint2; break;
            case 23: TXCommand = Get_Radiator4Temperature1; break;
            case 24: TXCommand = Get_Radiator4Temperature2; break;

            case 25: TXCommand = Get_Radiator5Setpoint1; break;
            case 26: TXCommand = Get_Radiator5Setpoint2; break;
            case 27: TXCommand = Get_Radiator5Temperature1; break;
            case 28: TXCommand = Get_Radiator5Temperature2; break;
            */
        }
        MY_SERIAL.print(" cmd = ");
        MY_SERIAL.println(TXCommand);
        ReportTXCommandValue(TXCommand, 0);

        zwaveMessageCounter = (zwaveMessageCounter + 1) % ZWAVE_MSG_COUNT;
        ZWAVE_TIMER.DurationInMillis = (zwaveMessageCounter == 0) ? ZWAVE_LONG_PERIOD : ZWAVE_SHORT_PERIOD;
        ZWAVE_RX_TIMER.Start();
    }

    // Run the thermostat loop
    THERM.Loop();

    // Wait if needed
    loopTime = millis() - loopStart;
    delay(LOOP_DELAY > loopTime ? LOOP_DELAY - loopTime : 1);
}



/**
* @brief The PC requests the Zuno TX command
*
*/
byte ZGetZtxCommand()
{
    return ztxCommand;
}

/**
* @brief The PC requests the Zuno TX value
*
*/
byte ZGetZtxValue()
{
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
    return zrxCommand;
}

/**
* @brief The PC requests the Zuno RX value (shouldn't happen)
*
*/
byte ZGetZrxValue()
{
    return zrxValue;
}

/**
* @brief The Zuno gets a Zuno RX command from the PC
*
*/
void ZSetZrxCommand(byte value)
{
    zrxCommand = value;
}

/**
* @brief The Zuno gets a Zuno RX value from the PC
*
*/
void ZSetZrxValue(byte value)
{
    zrxValue = value;
}



/**
* @brief Zwave Getter for Real Temperature
*
*/
word ZGetRealTemperature()
{
    return EncodeSensorReading(SensorTemperature);
}

/**
* @brief Zwave Getter for Real Humidity
*
*/
word ZGetRealHumidity()
{
    return EncodeSensorReading(SensorHumidity);
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
    switch (callback_data.type) {
        case ZUNO_CHANNEL1_GETTER: callback_data.param.bParam = ZGetZtxCommand(); break;
        case ZUNO_CHANNEL1_SETTER: ZSetZtxCommand(callback_data.param.bParam); break;

        case ZUNO_CHANNEL2_GETTER: callback_data.param.bParam = ZGetZtxValue(); break;
        case ZUNO_CHANNEL2_SETTER: ZSetZtxValue(callback_data.param.bParam); break;

        case ZUNO_CHANNEL3_GETTER: callback_data.param.bParam = ZGetZrxCommand(); break;
        case ZUNO_CHANNEL3_SETTER: ZSetZrxCommand(callback_data.param.bParam); break;

        case ZUNO_CHANNEL4_GETTER: callback_data.param.bParam = ZGetZrxValue(); break;
        case ZUNO_CHANNEL4_SETTER: ZSetZrxValue(callback_data.param.bParam); break;
        
        case ZUNO_CHANNEL5_GETTER: callback_data.param.wParam = ZGetRealTemperature(); break;
        case ZUNO_CHANNEL6_GETTER: callback_data.param.wParam = ZGetRealHumidity(); break;
        
        default: break;
    }
}
