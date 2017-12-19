#include "led_control.h"

LedClass LED0(PIN_LED_R1, PIN_LED_G1, PIN_LED_B1);
LedClass LED1(PIN_LED_R2, PIN_LED_G2, PIN_LED_B2);
LedClass LED2(PIN_LED_R3, PIN_LED_G3, PIN_LED_B3);
TimerClass BLINK_TIMER(1500);
TimerClass FLASH_TIMER(100);
TimerClass ANIMATION_TIMER(250);
TimerClass TEMP_CHANGE_TIMER(30000);

LedControlClass::LedControlClass(SensorClass* sensor, BoilerClass* boiler, ThermostatClass* thermostat):
    SENSOR(sensor), BOILER(boiler), THERM(thermostat)
{
    ledBlinkState = false;
    ledColor = COLOR_BLACK;
    ledColor0 = COLOR_BLACK;
    ledColor1 = COLOR_BLACK;
    ledColor2 = COLOR_BLACK;
    flashColor = COLOR_BLACK;
    flashCounter = 0;
    animationDirection = 0;
    animationIndex = 0;
    lastTemp = 0;
    power = true;
}

void LedControlClass::SetPower(bool value)
{
    power = value;
}

void LedControlClass::Init()
{
    LED0.Init();
    LED1.Init();
    LED2.Init();
    lastTemp = SENSOR->GetTemperature();
}

void LedControlClass::FlashEnqueue(byte color)
{
    if (flashQueueSize < FLASH_QUEUE_LEN) {
        flashQueue[flashQueueSize] = color;
        flashQueueSize++;
    }
}

byte LedControlClass::FlashDequeue()
{
    byte color = flashQueue[0];
    flashQueueSize--;
    for (byte i = 0; i < FLASH_QUEUE_LEN; i++) {
        if (i < flashQueueSize)
            flashQueue[i] = flashQueue[i + 1];
        else
            flashQueue[i] = COLOR_BLACK;
    }
    return color;
}

void LedControlClass::DisplayColorAll(byte color0, byte color1, byte color2)
{
    if (!power) {
        color0 = COLOR_BLACK;
        color1 = COLOR_BLACK;
        color2 = COLOR_BLACK;
    }
    LED0.DisplayColor(color0);
    LED1.DisplayColor(color1);
    LED2.DisplayColor(color2);
}

void LedControlClass::SetFlash(byte color)
{
    FlashEnqueue(color);
}

void LedControlClass::DoFlash(byte color)
{
    FLASH_TIMER.Start();
    flashCounter = FLASHES;
    flashColor = color;
}

void LedControlClass::SetBlinkingState()
{
    bool state = BOILER->GetBoilerState();
    if (BLINK_TIMER.IsActive != state) {
        BLINK_TIMER.IsActive = state;
        if (BLINK_TIMER.IsActive)
            BLINK_TIMER.Start();
    }
}

/**
 * @brief Controls LED animation
 * 
 * @param direction 0=off, -1=down, 1=up
 * @param period millis between each frame of the animation
 */
void LedControlClass::StartAnimation(int direction, int period)
{
    if (animationDirection == direction && ANIMATION_TIMER.DurationInMillis == period) return;
    animationDirection = direction;
    ANIMATION_TIMER.DurationInMillis = period;
    if (animationDirection != 0) {
        ANIMATION_TIMER.Start();
    }
}

/**
 * @brief Set the LED animation according to temperature change
 * 
 */
void LedControlClass::SetAnimationState()
{
    if (TEMP_CHANGE_TIMER.IsElapsed()) {
        TEMP_CHANGE_TIMER.Start();
        float delta = SENSOR->GetTemperature() - lastTemp;
        if (delta != 0) {
            int period = 100 / abs(delta);
            if (period < 100) period = 100;
            if (period > 1000) period = 1000;
            if (delta > 0)
                StartAnimation(1, period);
            else if (delta < 0)
                StartAnimation(-1, period);
        }
        else
            StartAnimation(0, 250);
        lastTemp = SENSOR->GetTemperature();
    }
}

void LedControlClass::DrawAll()
{
    // Toggle boiler blink
    if (BLINK_TIMER.IsActive && BLINK_TIMER.IsElapsed()) {
        ledBlinkState = !ledBlinkState;
        if (ledBlinkState)
            SetFlash(BOILER_BLINK_COLOR);
        BLINK_TIMER.Start();
    }

    // Set base color according to mode
    switch (THERM->GetMode()) {
        case Frost:  ledColor = COLOR_BLUE; break;
        case Absent: ledColor = COLOR_CYAN; break;
        case Night:  ledColor = COLOR_MAGENTA; break;
        case Day:    ledColor = COLOR_GREEN; break;
        case Warm:   ledColor = COLOR_YELLOW; break;
    }

    // Flash LED
    if (!FLASH_TIMER.IsActive && flashQueueSize > 0) {
        DoFlash(FlashDequeue());
    }
    if (FLASH_TIMER.IsActive) {
        ledColor = COLOR_BLACK;
        if (!FLASH_TIMER.IsElapsed()) {
            if (flashCounter == 2)
                ledColor = flashColor;
        }
        else {
            flashCounter--;
            if (flashCounter > 0)
                FLASH_TIMER.Start();
        }
    }

    // Draw animations
    ledColor0 = ledColor;
    ledColor1 = ledColor;
    ledColor2 = ledColor;
    if (animationDirection != 0) {
        if (ANIMATION_TIMER.IsElapsed()) {
            ANIMATION_TIMER.Start();
            animationIndex += animationDirection;
            if (animationIndex < 0)
                animationIndex = LED_COUNT;
            if (animationIndex >= LED_COUNT + 1)
                animationIndex = 0;
        }
        switch (animationIndex) {
            case 1: ledColor0 = COLOR_BLACK; break;
            case 2: ledColor1 = COLOR_BLACK; break;
            case 3: ledColor2 = COLOR_BLACK; break;
            case 0: // no break
            default: break;
        }
    }

    DisplayColorAll(ledColor0, ledColor1, ledColor2);
}
