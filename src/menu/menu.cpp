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

#define LINE_Y(i) ((i) * 10)

int gameIndex = 0;
const int MENU_COUNT = 4;

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
    }
    if (isPressed(BTN_DOWN)) {
        gameIndex++;
        if (gameIndex >= MENU_COUNT) {
            gameIndex = 0;
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

    display.setTextSize(2);
    display.setCursor(36, 0);
    display.println("Mono");
    display.setTextSize(1);
    display.setCursor(58, 16);
    display.println("Deck");

    display.drawLine(0, 28, 127, 28, SH110X_WHITE);
    
    const char* gameNames[] = {
        "Snake",
        "Breakout",
        "Pong",
        "Tetricore"
    };
    
    display.setTextSize(1);

    for (int i = 0; i < MENU_COUNT; i++) {
        display.setCursor(0, 32 + LINE_Y(i));
        if (i == gameIndex) {
            display.print("> ");
        } else {
            display.print("  ");
        }
        display.println(gameNames[i]);
    }
    display.display();
}