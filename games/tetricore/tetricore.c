#include "tetricore.h"
#include "drivers/sh110x/sh110x.h"
#include "core/app_state.h"
#include "core/engine.h"
#include "core/input.h"

void initTetricore(void) {
}

void updateTetricore(void) {
    if (isPressed(BTN_B)) {
        requestExitToMenu();
    }
}

void drawTetricore(void) {
    sh110x_clear();
    sh110x_draw_text(20, 20, "TETRICORE", 1);
    sh110x_draw_text(10, 40, "B: exit", 1);
    sh110x_update();
}

