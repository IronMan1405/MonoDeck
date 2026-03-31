#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "platform/platform_config.h"
#include "platform/platform_init.h"
#include "platform/platform_time.h"

#include "drivers/sh110x/sh110x.h"

#include "core/app_state.h"
#include "app_registry/app_registry.h"
#include "core/input.h"
#include "core/engine.h"
#include "core/boot.h"

#include "menu/menu.h"

int main() {
    platform_init();

    sleep_ms(5000);

    printf("Monodeck v2 booting...\n");

    uint8_t dummy = 0x00;
    int ret = i2c_write_blocking(I2C_PORT, DISPLAY_I2C_ADDRESS, &dummy, 1, false);

    if (ret < 0) {
        printf("OLED NOT responding on I2C\n");
    } else {
        printf("OLED ACKed on I2C\n");
    }
    
    sh110x_init();
    sh110x_clear();
    sh110x_update();

    initInput();
    initBoot();

    while (1) {
        if (!platform_frame_ready()) continue;

        updateInput();

        switch (currentState) {
        case BOOT:
            updateBoot();
            drawBoot();
            break;
        case MENU:
            updateMenu();
            drawMenu();
            break;
        case IN_GAME:
            tickGame();
            break;
        }

        if (gameWantsExit) {
            gameWantsExit = false;
            requestExitToMenu();
            initMenu();
            currentState = MENU;
        }
    }

    return 0;
}
