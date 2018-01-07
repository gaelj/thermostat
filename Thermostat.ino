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


// Custom sensor to retrieve temperature as a word (2 bytes)
#define ZUNO_SENSOR_MULTILEVEL_TEMPERATURE_WORD(GETTER) ZUNO_SENSOR_MULTILEVEL (ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE, SENSOR_MULTILEVEL_SCALE_CELSIUS, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_TWO_DECIMALS, GETTER)
#define ZUNO_SENSOR_MULTILEVEL_HUMIDITY_WORD(GETTER)    ZUNO_SENSOR_MULTILEVEL (ZUNO_SENSOR_MULTILEVEL_TYPE_RELATIVE_HUMIDITY, SENSOR_MULTILEVEL_SCALE_PERCENTAGE_VALUE, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_TWO_DECIMALS, GETTER)

ZUNO_SETUP_DEBUG_MODE(DEBUG_ON);

// Zwave channels: 1 channel to get/set the command id,
//                 1 channel to (get/)set the command value,
//                 1 channel to get the real temperature,
//                 1 channel to get the real humidity
ZUNO_SETUP_CHANNELS(ZUNO_SWITCH_MULTILEVEL(ZGetZtxCommand, ZSetZtxCommand),
                    ZUNO_SWITCH_MULTILEVEL(ZGetZtxValue, ZSetZtxValue),
                    ZUNO_SWITCH_MULTILEVEL(ZGetZrxValue, ZSetZrxValue),
                    ZUNO_SWITCH_MULTILEVEL(ZGetZrxValue, ZSetZrxValue),
                    ZUNO_SENSOR_MULTILEVEL_TEMPERATURE_WORD(ZGetRealTemperature),
                    ZUNO_SENSOR_MULTILEVEL_HUMIDITY_WORD(ZGetRealHumidity)
);

// Zwave associations: 1 group to set boiler relay on/off
// (association to the relay must be setup in the Zwave network master)
ZUNO_SETUP_ASSOCIATIONS(ZUNO_ASSOCIATION_GROUP_SET_VALUE);


// Create objects
static TimerClass ZWAVE_RX_TIMER(0);
static TimerClass SENSOR_TIMER(READ_SENSOR_PERIOD);
static TimerClass MODE_SET_DELAY_TIMER(MODE_SET_DELAY_PERIOD);
PID PIDREG;
// static PID_ATune atune;
// static AutoPidClass AUTOPID(&pid, &atune, &SETTINGS, &MODE);

radiator_s Radiators[RADIATOR_COUNT];
params_s Prm;

ButtonActions buttonAction;
unsigned long loopStart;
unsigned long loopTime;
int TXCommand = 0;
byte resetCnt = 0;

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
    Remote_InitParameters();
    OledDisplay_Init();
    Thermostat_Init();

    //if (!SETTINGS.RestoreSettings()) {
    //SETTINGS.LoadDefaults();
    //if (!SETTINGS.PersistSettings())
    //    settingsError = true;
    //}

    //AUTOPID.ApplySettings();
    //AUTOPID.SetAutoTune(1);
    ZWAVE_RX_TIMER.Start();
}

/**
* @brief Main loop function
*
*/
void loop()
{
    loopStart = millis();

    if (SENSOR_TIMER.IsElapsedRestart())
        ReadSensor();

    // Handle button presses
    buttonAction = ReadButtons();
    if (buttonAction == Button12 || (!Prm.IlluminationPower && buttonAction != NoButtonAction)) {
        Prm.IlluminationPower = !Prm.IlluminationPower;
        //LEDS.SetPower(Prm.IlluminationPower);
        OledDisplay_SetPower(Prm.IlluminationPower);
    }
    else if (buttonAction == Button1) {
        MODE_SET_DELAY_TIMER.Start();
        ThermostatMode newMode = ThermostatMode((byte(Prm.CurrentThermostatMode) + 1) % THERMOSTAT_MODE_COUNT);
        Prm.CurrentThermostatMode = newMode;
    }
    else if (buttonAction == Button2) {
        OledDisplay_ShowNextPage();
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
    OledDisplay_DrawDisplay();

    // Process any received command / value ZWave input
    if (ZWAVE_RX_TIMER.IsActive && ZWAVE_RX_TIMER.IsElapsed()) {

#ifdef LOGGING_ACTIVE
        MY_SERIAL.print("TXCmd: ");
        MY_SERIAL.println(TXCommand);
        MY_SERIAL.print("RXCmd: ");
        MY_SERIAL.println(zrxCommand);
        MY_SERIAL.print("RXVal: ");
        MY_SERIAL.println(zrxValue);
#endif // LOGGING_ACTIVE

        if (zrxCommand == TXCommand) {
            if (zrxCommand != 0 && zrxCommand != 99) {
                Remote_ProcessCommandValue(Commands(zrxCommand), zrxValue);
            }

            // Update Zwave values
            ZWAVE_RX_TIMER.DurationInMillis = (TXCommand == Get_Mode) ? ZWAVE_LONG_PERIOD : ZWAVE_SHORT_PERIOD;

            TXCommand = (currentCommand % ZWAVE_MSG_COUNT) + 1;

            if (TXCommand == Get_Mode) {
                ReportTemperature();
                ReportHumidity();
            }

            ReportTXCommandValue(TXCommand, 0);
            ZWAVE_RX_TIMER.Start();
#ifdef LOGGING_ACTIVE
            MY_SERIAL.print("TXCmd: ");
            MY_SERIAL.println(TXCommand);
            MY_SERIAL.print("Timer duration: ");
            MY_SERIAL.println(float(ZWAVE_RX_TIMER.DurationInMillis) / 1000);
#endif // LOGGING_ACTIVE

        }
        else {
#ifdef LOGGING_ACTIVE
            MY_SERIAL.println("RST");
#endif // LOGGING_ACTIVE
            int rstId = 0;
            resetCnt++;
            if (resetCnt > 1) {
                rstId = 99;
                resetCnt = 0;
            }
            //ReportRXCommandValue(rstId, rstId);
            ReportTXCommandValue(rstId, rstId);
            TXCommand = rstId;
            ZWAVE_RX_TIMER.DurationInMillis = 2 * ZWAVE_SHORT_PERIOD;
            ZWAVE_RX_TIMER.Start();
        }
    }

    // Run the thermostat loop
    if (currentPage != OLED_PAGE_COUNT)
        Thermostat_Loop();

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
