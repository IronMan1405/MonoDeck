#include "pico/stdlib.h"
#include "platform_time.h"

#define FRAME_TIME 50   // 50ms = 20 FPS

static uint32_t lastFrameTime = 0;

uint32_t platform_millis(void) {
    return to_ms_since_boot(get_absolute_time());
}

bool platform_frame_ready(void) {
    uint32_t now = platform_millis();

    if (now - lastFrameTime >= FRAME_TIME) {
        lastFrameTime = now;
        return true;
    }
    return false;
}