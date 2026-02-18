#include "input.h"
#include "drivers/buttons/buttons.h"

static bool current[BTN_COUNT];
static bool prev[BTN_COUNT];

void initInput(void) {
    buttons_init();

    for (int i = 0; i < BTN_COUNT; i++) {
        current[i] = false;
        prev[i] = false;
    }
}

void updateInput(void) {

    for (int i = 0; i < BTN_COUNT; i++) {
        prev[i] = current[i];
        current[i] = buttons_read(i);
    }
}

bool isPressed(Button x) {
    return current[x] && !prev[x];
}

bool isHeld(Button x) {
    return current[x];
}