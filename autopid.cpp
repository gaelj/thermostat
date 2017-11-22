#include "autopid.h"

/**
 * @brief Constructor: Set default values
 * 
 * @param pid 
 * @param atune 
 * @param settings 
 */
AutoPidClass::AutoPidClass(PID* pid, PID_ATune* atune, SettingsClass* settings, ThermostatModeClass* mode):
        myPID(pid), aTune(atune), SETTINGS(settings), MODE(mode) {
    ATuneModeRemember = 2;
    tuning = false;    
    Input = 0;
    Output = 0;
}

/**
 * @brief Initialise the PID and Autotune objects
 * 
 */
void AutoPidClass::ApplySettings() {
    Serial.println("Create PID & autotune");

    //Tell the PID to range between 0 and the full window size
    myPID->Create(&Input, &Output, 
        SETTINGS->TheSettings->Kp, SETTINGS->TheSettings->Ki, SETTINGS->TheSettings->Kd,
        P_ON_E, DIRECT, 0, SETTINGS->TheSettings->SampleTime);

    aTune->Create(&Input, &Output);

    //Setup the pid
    myPID->SetMode(AUTOMATIC);
}

/**
 * @brief Start or stop auto tune
 * 
 */
void AutoPidClass::ChangeAutoTune() {
    if (!tuning) {
        //Set the output to the desired starting frequency.
        Output = SETTINGS->TheSettings->ATuneStartValue;
        aTune->SetNoiseBand(SETTINGS->TheSettings->ATuneNoise);
        aTune->SetOutputStep(SETTINGS->TheSettings->ATuneStep);
        aTune->SetLookbackSec((int)SETTINGS->TheSettings->ATuneLookBack);
        AutoTuneHelper(true);
        tuning = true;
    } else {
        //cancel autotune
        aTune->Cancel();
        tuning = false;
        AutoTuneHelper(false);
    }
}

/**
 * @brief Saves the PID mode if true, restores it if false
 * 
 * @param start 
 */
void AutoPidClass::AutoTuneHelper(bool start) {
    if (start) 
        ATuneModeRemember = myPID->GetMode();
    else
        myPID->SetMode(ATuneModeRemember);
}

/**
 * @brief Print an information string on Serial
 * 
 */
void AutoPidClass::SerialPrintInfoString() {
    Serial.print("s: "); Serial.print(SETTINGS->GetSetPoint(MODE->CurrentThermostatMode)); Serial.print(" ");
    Serial.print("i: "); Serial.print(Input); Serial.print(" ");
    Serial.print("o: "); Serial.print(Output); Serial.print(" ");
    if (tuning) {
        Serial.println("tuning mode");
    } else {
        Serial.print("kp: "); Serial.print(myPID->GetKp()); Serial.print(" ");
        Serial.print("ki: "); Serial.print(myPID->GetKi()); Serial.print(" ");
        Serial.print("kd: "); Serial.print(myPID->GetKd()); Serial.println();
    }
}

/**
 * @brief Start or stop autotuning
 * 
 * @param b 0 to stop, or 1 to start
 */
void AutoPidClass::SetAutoTune(char b) {
    if ((b == '1' && !tuning) || (b != '1' && tuning))
      ChangeAutoTune();
}

/**
 * @brief Run autotune or get the values from the PID
 * 
 * @param input   the PID input value
 * @return float  the PID output value
 */
float AutoPidClass::Loop(float input) {
    Serial.println("autoPid loop");
    Input = input;
    unsigned long now = millis();

    if (tuning) {
        Serial.println("autoPid tuning");
        byte val = 0;
        val = aTune->Loop();
        if (val != 0) {
            tuning = false;
        }
        if (!tuning) {
            Serial.println("autoPid done");
            //we're done, set the tuning parameters
            float ki = aTune->GetKi();
            float kp = aTune->GetKp();
            float kd = aTune->GetKd();

            if (SETTINGS->TheSettings->Kp != kp || SETTINGS->TheSettings->Ki != ki || SETTINGS->TheSettings->Kd != kd) {
                SETTINGS->TheSettings->Kp = kp;
                SETTINGS->TheSettings->Ki = ki;
                SETTINGS->TheSettings->Kd = kd;
                SETTINGS->PersistSettings();
            }
            myPID->SetTunings(kp, ki, kd, myPID->pOn);
            AutoTuneHelper(false);
        }
    }
    else {
        Serial.println("autoPid compute");
        myPID->Compute(SETTINGS->GetSetPoint(MODE->CurrentThermostatMode));
    }
    
    SerialPrintInfoString();
    return Output;
}
