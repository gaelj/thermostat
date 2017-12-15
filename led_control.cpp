#include "led_control.h"

LedControlClass::LedControlClass(LedClass* led0, LedClass* led1, LedClass* led2,
    TimerClass* flashTimer, TimerClass* blinkTimer, TimerClass* animationTimer) :
    LED0(led0), LED1(led1), LED2(led2),
    FLASH_TIMER(flashTimer), BLINK_TIMER(blinkTimer), ANIMATION_TIMER(animationTimer)
{

}

void LedControlClass::Init()
{
    LED0->Begin();
    LED1->Begin();
    LED2->Begin();
}

void LedControlClass::DisplayColorAll(byte color0, byte color1, byte color2)
{
    LED0->DisplayColor(color0);
    LED1->DisplayColor(color1);
    LED2->DisplayColor(color2);
}

void LedControlClass::FlashAll(byte color)
{
    FLASH_TIMER->Init();
    flashCounter = FLASHES;
    flashColor = color;
}

void LedControlClass::SetBlinkingState(bool state)
{
    if (BLINK_TIMER->IsActive != state) {
        BLINK_TIMER->IsActive = state;
        if (BLINK_TIMER->IsActive)
            BLINK_TIMER->Init();
    }
}

/**
 * @brief Controls LED animation
 * @param direction 0=off, -1=down, 1=up
 */
void LedControlClass::SetAnimation(int direction)
{
    if (animationDirection == direction) return;
    animationDirection = direction;
    if (animationDirection != 0) {
        ANIMATION_TIMER->Init();
    }
}

void LedControlClass::DrawAll(ThermostatMode mode)
{
    // Toggle blink
    if (BLINK_TIMER->IsActive && BLINK_TIMER->IsElapsed()) {
        ledBlinkState = !ledBlinkState;
        if (ledBlinkState)
            FlashAll(COLOR_RED);
        BLINK_TIMER->Init();
    }

    // Set base color according to mode
    switch (mode) {
        case Frost:  ledColor = COLOR_BLUE; break;
        case Absent: ledColor = COLOR_CYAN; break;
        case Night:  ledColor = COLOR_MAGENTA; break;
        case Day:    ledColor = COLOR_GREEN; break;
        case Warm:   ledColor = COLOR_YELLOW; break;
    }

    // Flash LED
    if (FLASH_TIMER->IsActive) {
        if (!FLASH_TIMER->IsElapsed()) {
            if (flashCounter % 2 == 1)
                ledColor = flashColor;
        }
        else {
            flashCounter--;
            if (flashCounter > 0)
                FLASH_TIMER->Init();
        }
    }

    // Draw animations
    ledColor0 = ledColor;
    ledColor1 = ledColor; 
    ledColor2 = ledColor;
    if (animationDirection != 0) {
        if (ANIMATION_TIMER->IsElapsed()) {
            ANIMATION_TIMER->Init();
            animationIndex += animationDirection;
            if (animationIndex < 0)
                animationIndex = LED_COUNT + 2 - 1;
            if (animationIndex >= LED_COUNT + 2)
                animationIndex = 0;
        }
        switch (animationIndex) {
            case 1: ledColor0 = COLOR_BLACK; break;
            case 2: ledColor1 = COLOR_BLACK; break;
            case 3: ledColor2 = COLOR_BLACK; break;
        }
    }

    DisplayColorAll(ledColor0, ledColor1, ledColor2);
}
