#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>

typedef enum {
    BTN_UP,
    BTN_DOWN,
    BTN_LEFT,
    BTN_RIGHT,
    BTN_B,
    BTN_A,
    BTN_SEL,
    BTN_START,
    BTN_COUNT
} Button;

void initInput();
void updateInput();

bool isPressed(Button x);
bool isHeld(Button x);

#endif