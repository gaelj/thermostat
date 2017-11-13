#include "autopid.h"


AutoPidClass::AutoPidClass(PID* pid, PID_ATune* atune): myPID(pid), aTune(atune) {
    ATuneModeRemember = 2;

    kp = SETTINGS.TheSettings.Kp;
    ki = SETTINGS.TheSettings.Ki;
    kd = SETTINGS.TheSettings.Kd;
    aTuneStep = SETTINGS.TheSettings.ATuneStep;
    aTuneNoise = SETTINGS.TheSettings.ATuneNoise;
    aTuneStartValue = SETTINGS.TheSettings.ATuneStartValue;
    aTuneLookBack = SETTINGS.TheSettings.ATuneLookBack;

    tuning = false;
}

void AutoPidClass::SetPointers(float* Input, float* Output, float* Setpoint, float WindowSize, unsigned long sampleTime) {
    //tell the PID to range between 0 and the full window size
    myPID->SetOutputLimits(0, WindowSize);
    myPID->SetSampleTime(sampleTime); //sample time in minutes
    myPID->Create(Input, Output, Setpoint, kp, ki, kd, P_ON_E, DIRECT);
    aTune->Create(Input, Output);
    Setup();
}

void AutoPidClass::Setup() {
    //Setup the pid 
    myPID->SetMode(AUTOMATIC);

    if (tuning) {
        tuning = false;
        ChangeAutoTune();
        tuning = true;
    }
    
    serialTime = 0;
}

void AutoPidClass::ChangeAutoTune() {
    if (!tuning) {
        //Set the output to the desired starting frequency.
        myPID->SetOutput(aTuneStartValue);
        aTune->SetNoiseBand(aTuneNoise);
        aTune->SetOutputStep(aTuneStep);
        aTune->SetLookbackSec((int)aTuneLookBack);
        AutoTuneHelper(true);
        tuning = true;
    } else {
        //cancel autotune
        aTune->Cancel();
        tuning = false;
        AutoTuneHelper(false);
    }
}

void AutoPidClass::AutoTuneHelper(bool start) {
    if (start) 
        ATuneModeRemember = myPID->GetMode();
    else
        myPID->SetMode(ATuneModeRemember);
}

void AutoPidClass::SerialPrintInfoString() {
    Serial.print("s: "); Serial.print(myPID->GetSetpoint()); Serial.print(" ");
    Serial.print("i: "); Serial.print(myPID->GetInput()); Serial.print(" ");
    Serial.print("o: "); Serial.print(myPID->GetOutput()); Serial.print(" ");
    if (tuning) {
        Serial.println("tuning mode");
    } else {
        Serial.print("kp: "); Serial.print(myPID->GetKp()); Serial.print(" ");
        Serial.print("ki: "); Serial.print(myPID->GetKi()); Serial.print(" ");
        Serial.print("kd: "); Serial.print(myPID->GetKd()); Serial.println();
    }
}

void AutoPidClass::SetAutoTune(char b) {
    if ((b == '1' && !tuning) || (b != '1' && tuning))
      ChangeAutoTune();
}

void AutoPidClass::Loop() {
    unsigned long now = millis();
    
    if (tuning) {
        byte val = aTune->Runtime();
        if (val != 0) {
            tuning = false;
        }
        if (!tuning) {
            //we're done, set the tuning parameters
            ki = aTune->GetKi();
            kp = aTune->GetKp();
            kd = aTune->GetKd();
            myPID->SetTunings(kp, ki, kd);
            AutoTuneHelper(false);
        }
    }
    else myPID->Compute();
    
    //send if it's time
    if (millis() > serialTime) {
        SerialPrintInfoString();
        serialTime += 500;
    }
}
