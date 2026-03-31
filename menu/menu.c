#include "menu.h"
#include "core/input.h"
#include "core/app_state.h"
#include "app_registry/app_registry.h"
#include "core/engine.h"
#include "drivers/sh110x/sh110x.h"
#include "platform/platform_config.h"
#include "assets/monodeck_logo_64x32.h"

#define LINE_Y(i) ((i) * 10)

#define VISIBLE_ITEMS 3
#define ITEM_HEIGHT 8
#define LIST_START_Y 36

static int selected = 0;
int scrollOffset = 0;
static bool canEnter = true;

void initMenu(void) {
    selected = 0;
    canEnter = false;
}

void updateMenu(void) {
    if (isPressed(BTN_UP)) {
        selected--;
        if (selected < 0) {
            selected = gameCount - 1;
        }
        if (selected < scrollOffset) {
            scrollOffset = selected;
        }
    }

    if (isPressed(BTN_DOWN)) {
        selected++;
        if (selected >= gameCount) {
            selected = 0;
        }
        if (selected >= scrollOffset + VISIBLE_ITEMS) {
            scrollOffset = selected - VISIBLE_ITEMS + 1;
        }
    }

    if (!isHeld(BTN_A)) {
        canEnter = true;
    }

    if (isPressed(BTN_A) && canEnter) {
        canEnter = false;
        launchGame(games[selected].name);
        return;
    }
}

void drawMenu(void) {
    sh110x_clear();

    sh110x_draw_bitmap(28, 0, monodeck_logo_64x32, MONODECK_LOGO_W2, MONODECK_LOGO_H2);
    sh110x_draw_line(0, 25, 127, 25);

    for (int i = 0; i < gameCount; i++) {
        int itemIndex = scrollOffset + i;

        if (itemIndex >= gameCount) break;
        if (itemIndex == selected) {
            sh110x_draw_text(0, 32 + LINE_Y(i), "> ", 1);
        }
        sh110x_draw_text(12, 32 + LINE_Y(i), games[itemIndex].name, 1);
    }

    sh110x_update();
}
