#include "game_registry.h"

#include "games/snake/snake.h"
#include "games/pong/pong.h"
#include "games/breakout/breakout.h"
#include "games/tetricore/tetricore.h"

Game games[] = {
    {   initSnake, updateSnake, drawSnake   },
    {   initBreakout, updateBreakout, drawBreakout   },
    {   initPong, updatePong, drawPong   },
    {   initTetricore, updateTetricore, drawTetricore   }
};

const int GAME_COUNT = sizeof(games) / sizeof(Game);