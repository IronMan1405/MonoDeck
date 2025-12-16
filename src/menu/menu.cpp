#include <Arduino.h>
#include "menu.h"
#include "core/app_state.h"
#include "core/display.h"
#include "core/input.h"

#include "games/snake.h"
#include "games/breakout.h"
#include "games/pong.h"
#include "games/tetricore.h"

#define LINE_Y(i) ((i) * 10)

int menuIndex = 0;
const int MENU_COUNT = 4;

static bool canEnter = true;

void initMenu() {
    menuIndex = 0;
    canEnter = false;
}

void updateMenu() {
    if (isPressed(BTN_UP)) {
        menuIndex--;
        if (menuIndex < 0) {
            menuIndex = MENU_COUNT - 1;
        }
    }
    if (isPressed(BTN_DOWN)) {
        menuIndex++;
        if (menuIndex >= MENU_COUNT) {
            menuIndex = 0;
        }
    }
    
    if (!isHeld(BTN_A)) {
        canEnter = true;
    }

    if (canEnter && isPressed(BTN_A)) {
        canEnter = false;

        switch (menuIndex) {
            case 0: 
                currentState = SNAKE; 
                initSnake();
                break;
            case 1:
                currentState = BREAKOUT;
                initBreakout();
                break;
            case 2:
                currentState = PONG;
                initPong();
                break;
            case 3:
                currentState = TETRICORE;
                initTetricore();
                break;
        }
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
    
    const char* menuItems[] = {
        "Snake",
        "Breakout",
        "Pong",
        "Tetricore"
    };
    
    display.setTextSize(1);

    for (int i = 0; i < MENU_COUNT; i++) {
        display.setCursor(0, 32 + LINE_Y(i));
        if (i == menuIndex) {
            display.print("> ");
        } else {
            display.print("  ");
        }
        display.println(menuItems[i]);
    }
    display.display();
}