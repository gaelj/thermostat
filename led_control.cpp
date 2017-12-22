#include "led_control.h"

#include "pinout.h"
#include "led.h"
#include "timer.h"
#include "zwave_encoding.h"
#include "settings.h"

LedClass LED0(PIN_LED_R1, PIN_LED_G1, PIN_LED_B1);
LedClass LED1(PIN_LED_R2, PIN_LED_G2, PIN_LED_B2);
LedClass LED2(PIN_LED_R3, PIN_LED_G3, PIN_LED_B3);

TimerClass BLINK_TIMER(LED_BLINK_PERIOD);
TimerClass FLASH_TIMER(LED_FLASH_PERIOD);
TimerClass ANIMATION_TIMER(LED_ANIMATION_STEP_PERIOD);
TimerClass TEMP_CHANGE_TIMER(LED_ANIMATION_TOTAL_PERIOD);

LedControlClass::LedControlClass(SensorClass* sensor, BoilerClass* boiler, ThermostatClass* thermostat):
    SENSOR(sensor), BOILER(boiler), THERM(thermostat)
{
    ledBlinkState = false;
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
    lastTemp = SENSOR->Temperature;
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

void LedControlClass::DisplayColorAll()
{
    LED0.DisplayColor(power ? ledColors[0] : COLOR_BLACK);
    LED1.DisplayColor(power ? ledColors[1] : COLOR_BLACK);
    LED2.DisplayColor(power ? ledColors[2] : COLOR_BLACK);
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
        float delta = SENSOR->Temperature - lastTemp;
        if (delta != 0) {
            int period = LED_ANIMATION_MIN_PERIOD / abs(delta);
            if (period < LED_ANIMATION_MIN_PERIOD) period = LED_ANIMATION_MIN_PERIOD;
            if (period > LED_ANIMATION_MAX_PERIOD) period = LED_ANIMATION_MAX_PERIOD;
            StartAnimation(delta > 0 ? 1 : -1, period);
        }
        else
            StartAnimation(0, LED_ANIMATION_STEP_PERIOD);
        lastTemp = SENSOR->Temperature;
    }
}

void LedControlClass::DrawAll()
{
    byte ledColor = COLOR_BLACK;
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
    if (flashCounter > 0) {
        if (FLASH_TIMER.IsElapsed()) {
            flashCounter--;
            if (flashCounter > 0)
                FLASH_TIMER.Start();
        }
        if (flashCounter == FLASHES - 1)
            ledColor = flashColor;
        else
            ledColor = COLOR_BLACK;
    }

    // Draw animations
    for (byte i = 0; i < LED_COUNT; i++)
        ledColors[i] = ledColor;

    if (animationDirection != 0) {
        if (ANIMATION_TIMER.IsElapsed()) {
            ANIMATION_TIMER.Start();
            animationIndex += animationDirection;
            if (animationIndex < 0)
                animationIndex = LED_COUNT;
            if (animationIndex > LED_COUNT)
                animationIndex = 0;
        }
        if (animationIndex > 0)
            ledColors[animationIndex - 1] = COLOR_BLACK;
    }

    DisplayColorAll();
}
