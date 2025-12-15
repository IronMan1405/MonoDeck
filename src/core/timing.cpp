#include "timing.h"

unsigned long lastFrameTime = 0;

bool frameReady() {
    unsigned long now = millis();

    if (now - lastFrameTime >= FRAME_TIME) {
        lastFrameTime = now;
        return true;
    }
    return false;
}