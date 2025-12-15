#include <Arduino.h>
#include "input.h"

#define PIN_UP 14 // D5
#define PIN_DOWN 12 // D6
#define PIN_LEFT 13 // D7
#define PIN_RIGHT 2 // D4
#define PIN_B 0 // D3
#define PIN_A 16 // D0

static const uint8_t buttonPins[BTN_COUNT] = {
  PIN_UP,
  PIN_DOWN,
  PIN_LEFT,
  PIN_RIGHT,
  PIN_A,
  PIN_B
};

static bool current[BTN_COUNT];
static bool prev[BTN_COUNT];

void initInput() {
    for (int i = 0; i < BTN_COUNT; i++) {
        pinMode(buttonPins[i], INPUT_PULLUP);
        current[i] = false;
        prev[i] = false;
    }
}

void updateInput() {
    for (int i = 0; i < BTN_COUNT; i++) {
        prev[i] = current[i];
        current[i] = (digitalRead(buttonPins[i]) == LOW);
    }
}

bool isPressed(Button x) {
    return current[x] && !prev[x];
}

bool isHeld(Button x) {
    return current[x];
}