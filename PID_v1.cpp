/**********************************************************************************************
 * Arduino PID Library - Version 1.1.1
 * by Brett Beauregard <br3ttb@gmail.com> brettbeauregard.com
 *
 * This Library is licensed under a GPLv3 License
 **********************************************************************************************/

#include <PID_v1.h>

/*Constructor (...)*********************************************************
 *    The parameters specified here are those for for which we can't set up
 *    reliable defaults, so we need to have the user set them.
 ***************************************************************************/
PID::PID(SettingsClass* settings): SETTINGS(settings) { }

void PID::Create(float* Input, float* Output, float* Setpoint, 
        float Kp, float Ki, float Kd, int POn, int ControllerDirection,
        float Min, float Max) {
    myOutput = Output;
    myInput = Input;
    mySetpoint = Setpoint;
    inAuto = false;

    // Set output limits
    if (Min >= Max) return;
    outMin = Min;
    outMax = Max;
    
    SetControllerDirection(ControllerDirection);
    SetTunings(Kp, Ki, Kd, POn);

    if (millis() > SETTINGS->TheSettings.SampleTime)
        lastTime = millis() - SETTINGS->TheSettings.SampleTime;
    else
        lastTime = 0;
}


/* Compute() **********************************************************************
 *     This, as they say, is where the magic happens.  this function should be called
 *   every time "void loop()" executes.  the function will decide for itself whether a new
 *   pid Output needs to be computed.  returns true when the output is computed,
 *   false when nothing has been done.
 **********************************************************************************/
bool PID::Compute() {
    Serial.println("Compute PID...");
    if (!inAuto) return false;
    unsigned long now = millis();
    unsigned long timeChange = (now - lastTime);

    if (timeChange >= SETTINGS->TheSettings.SampleTime) {
        //Compute all the working error variables
        float input = *myInput;
        float error = *mySetpoint - input;
        float dInput = (input - lastInput);
        outputSum += (ki * error);

        //Add Proportional on Measurement, if P_ON_M is specified
        if (!pOnE) outputSum -= kp * dInput;

        if (outputSum > outMax) outputSum = outMax;
        else if (outputSum < outMin) outputSum = outMin;

        //Add Proportional on Error, if P_ON_E is specified
        float output;
        if (pOnE) output = kp * error;
        else output = 0;

        //Compute Rest of PID Output
        output += outputSum - kd * dInput;

        if (output > outMax) output = outMax;
        else if (output < outMin) output = outMin;
        *myOutput = output;

        //Remember some variables for next time
        lastInput = input;
        lastTime = now;

        return true;
    }
    else return false;
}

/* SetTunings(...)*************************************************************
 * This function allows the controller's dynamic performance to be adjusted.
 * it's called automatically from the constructor, but tunings can also
 * be adjusted on the fly during normal operation
 ******************************************************************************/
void PID::SetTunings(float Kp, float Ki, float Kd, int POn) {
    if (Kp < 0 || Ki < 0 || Kd < 0) return;

    pOn = POn;
    pOnE = POn == P_ON_E;

    dispKp = Kp; dispKi = Ki; dispKd = Kd;

    float SampleTimeInSec = ((float)SETTINGS->TheSettings.SampleTime) / 1000;
    kp = Kp;
    ki = Ki * SampleTimeInSec;
    kd = Kd / SampleTimeInSec;

    if (controllerDirection == REVERSE) {
        kp = (0 - kp);
        ki = (0 - ki);
        kd = (0 - kd);
    }
}

/* SetMode(...)****************************************************************
 * Allows the controller Mode to be set to manual (0) or Automatic (non-zero)
 * when the transition from manual to auto occurs, the controller is
 * automatically initialized
 ******************************************************************************/
void PID::SetMode(int Mode) {
    bool newAuto = (Mode == AUTOMATIC);
    if (newAuto && !inAuto) {  /*we just went from manual to auto*/
        Initialize();
    }
    inAuto = newAuto;
}

/* Initialize()****************************************************************
 *	does all the things that need to happen to ensure a bumpless transfer
 *  from manual to automatic mode.
 ******************************************************************************/
void PID::Initialize() {
    outputSum = *myOutput;
    lastInput = *myInput;
    if (outputSum > outMax) outputSum = outMax;
    else if (outputSum < outMin) outputSum = outMin;
}

/* SetControllerDirection(...)*************************************************
 * The PID will either be connected to a DIRECT acting process (+Output leads
 * to +Input) or a REVERSE acting process(+Output leads to -Input.)  we need to
 * know which one, because otherwise we may increase the output when we should
 * be decreasing.  This is called from the constructor.
 ******************************************************************************/
void PID::SetControllerDirection(int Direction) {
    if (inAuto && Direction != controllerDirection) {
        kp = (0 - kp);
        ki = (0 - ki);
        kd = (0 - kd);
    }
    controllerDirection = Direction;
}

/* Status Funcions*************************************************************
 * Just because you set the Kp=-1 doesn't mean it actually happened.  these
 * functions query the internal state of the PID.  they're here for display
 * purposes.  this are the functions the PID Front-end uses for example
 ******************************************************************************/
float PID::GetKp() { return  dispKp; }
float PID::GetKi() { return  dispKi; }
float PID::GetKd() { return  dispKd; }
int PID::GetMode() { return  inAuto ? AUTOMATIC : MANUAL; }
int PID::GetDirection() { return controllerDirection; }
