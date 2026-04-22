#include "app_registry.h"
#include "games/snake/snake.h"
#include "games/pong/pong.h"
#include "games/breakout/breakout.h"
#include "games/tetricore/tetricore.h"
#include "games/flappy/flappy.h"
#include "games/conwaysgol/conwaysgol.h"

#include <string.h>

Game games[] = {
    {"Snake", initSnake, updateSnake, drawSnake},
    {"Pong", initPong, updatePong, drawPong},
    {"Breakout", initBreakout, updateBreakout, drawBreakout},
    {"Tetricore", initTetricore, updateTetricore, drawTetricore},
    {"Flappy", initFlappy, updateFlappy, drawFlappy},
    {"Conway's GOL", initConway, updateConway, drawConway}
};

const int gameCount = sizeof(games) / sizeof(games[0]);

const Game* registry_find(const char* name) {
    for (int i = 0; i < gameCount; i++) {
        if (strcmp(games[i].name, name) == 0) {
            return &games[i];
        }
    }
    return NULL;
}