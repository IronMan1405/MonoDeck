#include "boot.h"
#include "display.h"
#include "app_state.h"
#include "input.h"
#include "timing.h"

#include "assets/monodeck_logo_128x64.h"

static uint32_t bootStart = 0;
static uint32_t BOOT_DURATION = 2500; // ms

void initBoot() {
    bootStart = millis();
}

void updateBoot() {
    if (isPressed(BTN_A) || isPressed(BTN_B)) {
        currentState = MENU;
        return;
    }
    if (millis() - bootStart >= BOOT_DURATION) {
        currentState = MENU;
    }
}

void drawBoot() {
    display.clearDisplay();
    
    display.drawBitmap(0, 0, monodeck_logo_128x64, MONODECK_LOGO_W, MONODECK_LOGO_H, SH110X_WHITE);

    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(108, 56);
    display.print("v1");

    display.display();
}