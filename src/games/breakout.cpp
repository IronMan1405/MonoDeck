#include "breakout.h"

#include "core/display.h"
#include "core/input.h"
#include "core/app_state.h"
#include "core/engine.h"

static bool canExit = true;

void initBreakout() {
    canExit = false;
    display.clearDisplay();
    display.display();
}

void updateBreakout() {
    if (!isHeld(BTN_B)) {
        canExit = true;
    }

    if (canExit && isPressed(BTN_B)) {
        canExit = false;
        requestExitToMenu();
        return;
    }
}

void drawBreakout() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print("Breakout");
    display.display();
}