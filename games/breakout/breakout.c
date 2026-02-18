#include "breakout.h"
#include "drivers/sh110x/sh110x.h"
#include "core/app_state.h"
#include "core/engine.h"
#include "core/input.h"

void initBreakout(void) {
}

void updateBreakout(void) {
    if (isPressed(BTN_B)) {
        requestExitToMenu();
    }
}

void drawBreakout(void) {
    sh110x_clear();
    sh110x_draw_text(20, 20, "BREAKOUT", 1);
    sh110x_draw_text(10, 40, "B: exit", 1);
    sh110x_update();
}

