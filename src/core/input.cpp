#include <Arduino.h>
#include "input.h"

#define BTN_UP 14 // D5
#define BTN_DOWN 12 // D6
#define BTN_B 0 // D3
#define BTN_A 16 // D8

bool upPressed, downPressed, bPressed, aPressed;
bool upPrev, downPrev, bPrev, aPrev;

void initInput() {
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_B, INPUT_PULLUP);
    pinMode(BTN_A, INPUT_PULLUP);
}

void updateInput() {
    bool upNow = !digitalRead(BTN_UP);
    bool downNow = !digitalRead(BTN_DOWN);
    bool bNow = !digitalRead(BTN_B);
    bool aNow = !digitalRead(BTN_A);

    upPressed = upNow && !upPrev;
    downPressed = downNow && !downPrev;
    bPressed = bNow && !bPrev;
    aPressed = aNow && !aPrev;

    upPrev = upNow;
    downPrev = downNow;
    bPrev = bNow;
    aPrev = aNow;
}

bool btnUp() {
    return upPressed;
}

bool btnDown() {
    return downPressed;
}

bool btnB() {
    return bPressed;
}

bool btnA() {
    return aPressed;
}