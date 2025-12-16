#include <Arduino.h>

#include "core/app_state.h"
#include "menu/menu.h"
#include "core/display.h"
#include "core/input.h"
#include "core/timing.h"
#include "core/engine.h"
#include "core/boot.h"

#include "games/snake.h"
#include "games/breakout.h"
#include "games/pong.h"
#include "games/tetricore.h"

void setup() {
  // put your setup code here, to run once:
  initDisplay();
  initInput();
  initBoot();
  currentState = BOOT;
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!frameReady()) {
    return;
  }

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
    case SNAKE:
      updateSnake();
      drawSnake();
      break;
    case PONG:
      updatePong();
      drawPong();
      break;
    case BREAKOUT:
      updateBreakout();
      drawBreakout();
      break;
    case TETRICORE:
      updateTetricore();
      drawTetricore();
      break;
  }
  if (gameWantsExit) {
    gameWantsExit = false;
    initMenu();
    currentState = MENU;
  }
}