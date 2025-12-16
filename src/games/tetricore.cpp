#include "tetricore.h"

#include "core/display.h"
#include "core/input.h"
#include "core/app_state.h"

static bool canExit = true;

void initTetricore() {
    canExit = false;
    display.clearDisplay();
    display.display();
}

void updateTetricore() {
    if (!isHeld(BTN_B)) {
        canExit = true;
    }

    if (canExit && isPressed(BTN_B)) {
        canExit = false;
        currentState = MENU;
        return;
    }
}

void drawTetricore() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print("Tetricore");
    display.display();
}