#include <Arduino.h>
#include "menu.h"
#include "core/app_state.h"
#include "core/display.h"
#include "core/input.h"
#include "core/engine.h"

#include "games/snake.h"
#include "games/breakout.h"
#include "games/pong.h"
#include "games/tetricore.h"

#include "assets/monodeck_logo_64x32.h"

#define LINE_Y(i) ((i) * 10)

#define VISIBLE_ITEMS 3
#define ITEM_HEIGHT 8
#define LIST_START_Y 36

int gameIndex = 0;
const int MENU_COUNT = 4;

int scrollOffset = 0;

static bool canEnter = true;

void initMenu() {
    gameIndex = 0;
    canEnter = false;
}

void updateMenu() {
    if (isPressed(BTN_UP)) {
        gameIndex--;
        if (gameIndex < 0) {
            gameIndex = MENU_COUNT - 1;
        }

        if (gameIndex < scrollOffset) {
            scrollOffset = gameIndex;
        }
    }
    if (isPressed(BTN_DOWN)) {
        gameIndex++;
        if (gameIndex >= MENU_COUNT) {
            gameIndex = 0;
        }

        if (gameIndex >= scrollOffset + VISIBLE_ITEMS) {
            scrollOffset = gameIndex - VISIBLE_ITEMS + 1;
        }
    }
    
    if (!isHeld(BTN_A)) {
        canEnter = true;
    }

    if (canEnter && isPressed(BTN_A)) {
        canEnter = false;
        launchGame(gameIndex);
        return;
    }
}

void drawMenu() {
    display.clearDisplay();

    display.drawBitmap(28, 0, monodeck_logo_64x32, MONODECK_LOGO_W2, MONODECK_LOGO_H2, SH110X_WHITE);

    display.drawLine(0, 25, 127, 25, SH110X_WHITE);
    
    const char* gameNames[] = {
        "Snake",
        "Breakout",
        "Pong",
        "Tetricore"
    };
    
    display.setTextSize(1);

    for (int i = 0; i < MENU_COUNT; i++) {
        int itemIndex = scrollOffset + i;

        if (itemIndex >= MENU_COUNT) break;

        display.setCursor(0, 32 + LINE_Y(i));

        if (itemIndex == gameIndex) {
            display.print("> ");
        } else {
            display.print("  ");
        }
        display.println(gameNames[itemIndex]);
    }
    display.display();
}