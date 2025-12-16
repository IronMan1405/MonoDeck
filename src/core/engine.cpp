#include "engine.h"

#include "core/app_state.h"
#include "core/game_registry.h"
#include "menu/menu.h"

#include "games/snake.h"
#include "games/breakout.h"
#include "games/pong.h"
#include "games/tetricore.h"

int currentGame = -1;
bool gameWantsExit = false;

void launchGame(int index) {
    currentGame = index;

    switch (index) {
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

}

void requestExitToMenu() {
    gameWantsExit = true;
}