#include "game_registry.h"

#include "games/snake.h"
#include "games/pong.h"
#include "games/breakout.h"
#include "games/tetricore.h"

Game games[] = {
    {   initSnake, updateSnake, drawSnake   },
    {   initBreakout, updateBreakout, drawBreakout   },
    {   initPong, updatePong, drawPong   },
    {   initTetricore, updateTetricore, drawTetricore   }
};

const int GAME_COUNT = sizeof(games) / sizeof(Game);