/**********************************************************************************************
 * Arduino PID Library - Version 2.0.0
 * by Brett Beauregard <br3ttb@gmail.com> brettbeauregard.com
 * modified by Gaël James <gaeljames@gmail.com>
 *
 * This Library is licensed under a GPLv3 License
 **********************************************************************************************/

#include <PID_v1.h>

/*Constructor (...)*********************************************************
 *    The parameters specified here are those for which we can't set up
 *    reliable defaults, so we need to have the user set them.
 ***************************************************************************/
PID::PID(SettingsClass* settings): SETTINGS(settings)
{
    lastInput = 0;
    lastOutput = 0;
    outputSum = 0;
}

void PID::Create(float Min, float Max)
{
    inAuto = false;

    // Set output limits
    if (Min >= Max) return;
    outMin = Min;
    outMax = Max;

    SetTunings();

    if (millis() > SETTINGS->TheSettings->SampleTime)
        lastTime = millis() - SETTINGS->TheSettings->SampleTime;
    else
        lastTime = 0;
}

/* Compute() **********************************************************************
 *     This, as they say, is where the magic happens. this function should be called
 *   every time "void loop()" executes. returns the output when the output is computed,
 *   or -1 when nothing has been done.
 **********************************************************************************/
float PID::Compute(const float input, const float mySetpoint)
{
    //Serial.println("Computing PID");

    if (!inAuto) return -1;
    unsigned long now = millis();
    //unsigned long timeChange = (now - lastTime);
    //if (timeChange < SETTINGS->TheSettings->SampleTime) return -1;

    //Compute all the working error variables
    float error = mySetpoint - input;
    float dInput = (input - lastInput);
    outputSum += (ki * error);

    //Add Proportional on Measurement, if P_ON_M is specified
    if (POn != P_ON_E) outputSum -= kp * dInput;

    BoundValue(&outputSum);

    //Add Proportional on Error, if P_ON_E is specified
    if (POn == P_ON_E) lastOutput = kp * error;
    else lastOutput = 0;

    //Compute Rest of PID Output
    lastOutput += outputSum - kd * dInput;

    BoundValue(&lastOutput);

    //Remember some variables for next time
    lastInput = input;
    lastTime = now;

    return lastOutput;
}

/* SetTunings(...)*************************************************************
 * This function allows the controller's dynamic performance to be adjusted.
 * it's called automatically from the constructor, but tunings can also
 * be adjusted on the fly during normal operation
 ******************************************************************************/
void PID::SetTunings()
{
    if (SETTINGS->TheSettings->Kp < 0 || SETTINGS->TheSettings->Ki < 0 || SETTINGS->TheSettings->Kd < 0)
        return;

    float SampleTimeInSec = float(SETTINGS->TheSettings->SampleTime) / 1000;
    kp = SETTINGS->TheSettings->Kp;
    ki = SETTINGS->TheSettings->Ki * SampleTimeInSec;
    kd = SETTINGS->TheSettings->Kd / SampleTimeInSec;

    if (ControllerDirection == REVERSE)
        InvertPidParameters();
}

/* SetMode(...)****************************************************************
 * Allows the controller Mode to be set to manual (0) or Automatic (non-zero)
 * when the transition from manual to auto occurs, the controller is
 * automatically initialized
 ******************************************************************************/
void PID::SetMode(int Mode)
{
    bool newAuto = (Mode == AUTOMATIC);
    if (newAuto && !inAuto)  //we just went from manual to auto
        Initialize();
    inAuto = newAuto;
}

/* Initialize()****************************************************************
 *    does all the things that need to happen to ensure a bumpless transfer
 *  from manual to automatic mode.
 ******************************************************************************/
void PID::Initialize()
{
    outputSum = lastOutput;
    lastInput = lastInput;
    BoundValue(&outputSum);
}

/**
* @brief Make sure the value falls between the outMin and outMax boundaries
* 
*/
void PID::BoundValue(float* value)
{
    if (*value > outMax) *value = outMax;
    else if (*value < outMin) *value = outMin;
}

/**
* @brief Make each PID parameter negative
*
*/
void PID::InvertPidParameters()
{
    kp = (0 - kp);
    ki = (0 - ki);
    kd = (0 - kd);
}
