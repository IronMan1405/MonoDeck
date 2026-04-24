#include <stdlib.h>
#include <stdio.h>

#include "clock.h"
#include "drivers/sh110x/sh110x.h"
#include "core/app_state.h"
#include "core/engine.h"
#include "core/input.h"
#include "platform/platform_time.h"
#include "platform/platform_storage.h"

static bool canExit = true;

ClockState clockState = CLOCK_RUNNING;

static uint32_t clock_base_seconds = 0;
static uint64_t clock_base_millis = 0;

static uint32_t current_seconds = 0;

static void split_time(uint32_t t, int *h, int *m, int *s);
static void drawClockRunning();
static void drawClockExit();

void initClock(void) {
    sh110x_clear();
    sh110x_update();

    clockState = CLOCK_RUNNING;

    canExit = false;

    clock_base_seconds = 0; // or load from storage later
    clock_base_millis = platform_millis();

    current_seconds = clock_base_seconds;
}

void updateClock(void) {
    switch (clockState) {
        case CLOCK_RUNNING:
            if (isPressed(BTN_B)) {
                clockState = CLOCK_EXIT_WARN;
            }
            uint64_t now = platform_millis();

            uint32_t elapsed_sec = (now - clock_base_millis) / 1000;

            current_seconds = clock_base_seconds + elapsed_sec;

            break;
        case CLOCK_EXIT_WARN:
            if (!isHeld(BTN_A)) {
                canExit = true;
            }
            if (canExit && isPressed(BTN_A)) {
                canExit = false;
                requestExitToMenu();
            }
            if (isPressed(BTN_B)) {
                clockState = CLOCK_RUNNING;
            }
            break;
    }
}

void drawClock(void) {
    sh110x_clear();

    switch (clockState) {
        case CLOCK_RUNNING:
            drawClockRunning();
            break;
        case CLOCK_EXIT_WARN:
            drawClockExit();
            break;
    }

    sh110x_update();
}

static void split_time(uint32_t t, int *h, int *m, int *s) {
    *s = t % 60;
    *m = (t / 60) % 60;
    *h = (t / 3600) % 24;
}


static void drawClockRunning() {
    int h, m, s;
    split_time(current_seconds, &h, &m, &s);

    char time_buf[9];
    snprintf(time_buf, sizeof(time_buf), "%02d:%02d:%02d", h, m, s);

    char date_buf[] = "24 APR";

    sh110x_draw_text(20, 20, time_buf, 2);
    sh110x_draw_text(25, 40, date_buf, 1);
}

static void drawClockExit() {
    sh110x_draw_text(5, 5, "Are you sure",1);
    sh110x_draw_text(5, 15, "you want to exit", 1);
    sh110x_draw_text(2, 56, "A: Proceed  B:Cancel", 1);
}
