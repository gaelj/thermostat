#ifndef PID_v1_h
#define PID_v1_h
#define LIBRARY_VERSION    2.0.0

#include <Arduino.h>
#include "settings.h"

class PID {
public:
    //Constants used in some of the functions below
    #define AUTOMATIC   1
    #define MANUAL      0

    #define DIRECT      0
    #define REVERSE     1
    
    #define P_ON_M      0
    #define P_ON_E      1

    // Commonly-used functions **************************************************************************
    PID(SettingsClass*);
    void Create(                          // * constructor.
        int, int,                         //   Initial tuning parameters are set here.
        float, float);                    // * clamps the output to a specific range. 0-255 by default, but
                                          //   it's likely the user will want to change this depending on
                                          //   the application
                                          //   sets the frequency, in Milliseconds, with which 
                                          //   the PID calculation is performed.  default is 100

    float Compute(const float,            // * performs the PID calculation.  it should be
        const float);                     //   called every time loop() cycles. ON/OFF and
                                          //   calculation frequency can be set using SetMode
                                          //   SetSampleTime respectively

    void SetMode(int);                    // * sets PID to either Manual (0) or Auto (non-0)

    // Display functions ****************************************************************
    int GetMode();                        //  inside the PID.
    int GetDirection();                   //

    int pOn;                              // P_ON_E or P_ON_M

private:
    SettingsClass* SETTINGS;
    void Initialize();
    void SetTunings(int);                 // * While most users will set the tunings once in the 
                                          //   constructor, this function gives the user the option
                                          //   of changing tunings during runtime for Adaptive control
                                          //   and proportional mode

    void SetControllerDirection(int);     // * Sets the Direction, or "Action" of the controller. DIRECT
                                          //   means the output will increase when error is positive. REVERSE
                                          //   means the opposite.  it's very unlikely that this will be needed
                                          //   once it is set in the constructor.


    float kp;                             // * (P)roportional Tuning Parameter
    float ki;                             // * (I)ntegral Tuning Parameter
    float kd;                             // * (D)erivative Tuning Parameter

    int controllerDirection;

    unsigned long lastTime;
    float outputSum, lastInput, lastOutput;

    float outMin, outMax;
    bool inAuto, pOnE;
};
#endif
