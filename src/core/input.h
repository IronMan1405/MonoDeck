#pragma once

enum Button {
    BTN_UP,
    BTN_DOWN,
    BTN_LEFT,
    BTN_RIGHT,
    BTN_B,
    BTN_A,
    BTN_COUNT
};

void initInput();
void updateInput();

bool isPressed(Button x);
bool isHeld(Button x);