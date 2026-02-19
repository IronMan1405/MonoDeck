#include "boot.h"
#include "drivers/sh110x/sh110x.h"
#include "core/app_state.h"
#include "core/input.h"
#include "platform/platform_time.h"
#include "assets/monodeck_logo_128x64.h"

static uint32_t bootStartTime = 0;
static uint32_t BOOT_DURATION = 2500;

void initBoot(void) {
    bootStartTime = platform_millis();
}

void updateBoot(void) {
    if (isPressed(BTN_A) || isPressed(BTN_B)) {
        currentState = MENU;
        return;
    }
    if (platform_millis() - bootStartTime > BOOT_DURATION)
    {
        currentState = MENU;
    }
}

void drawBoot(void) {
    sh110x_clear();
    // sh110x_draw_text(20, 20, "Monodeck v2", 1);
    sh110x_draw_bitmap(0, 0, monodeck_logo_128x64, MONODECK_LOGO_W, MONODECK_LOGO_H);
    sh110x_update();
}