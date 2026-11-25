#include <stdbool.h>

#include "scope.h"
#include "drivers/sh110x/sh110x.h"
#include "core/engine.h"
#include "core/input.h"

typedef struct {
    bool frozen;
    bool canExit;
} ScopeState;

static ScopeState scope;

void initScope(void) {
    scope.frozen = false;
    scope.canExit = false;

    sh110x_clear();
    sh110x_update();
}

void updateScope(void) {
    if (!isHeld(BTN_B)) {
        scope.canExit = true;
    }

    if (scope.canExit && isPressed(BTN_B)) {
        scope.canExit = false;
        requestExitToMenu();
        return;
    }

    if (isPressed(BTN_A)) {
        scope.frozen = !scope.frozen;
    }
}

void drawScope(void) {
    sh110x_clear();

    // Header
    sh110x_draw_text(2, 0, "SCOPE", 1);
    sh110x_draw_text(88, 0, scope.frozen ? "HOLD" : "LIVE", 1);

    // Plot boundary
    sh110x_draw_rect(0, 10, 128, 43);

    // Horizontal grid divisions
    for (int y = 20; y <= 40; y += 10) {
        sh110x_draw_line(1, y, 126, y);
    }

    // Vertical grid divisions
    for (int x = 16; x <= 112; x += 16) {
        sh110x_draw_line(x, 11, x, 51);
    }

    // Placeholder until ADC acquisition is implemented
    sh110x_draw_text(30, 25, "NO SIGNAL", 1);

    // Footer
    sh110x_draw_text(2, 55, "A:Hold       B:Exit", 1);

    sh110x_update();
}