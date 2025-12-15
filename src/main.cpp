#include <Arduino.h>
#include "core/app_state.h"
#include "menu/menu.h"
#include "core/display.h"
#include "core/input.h"

void setup() {
  // put your setup code here, to run once:
  initDisplay();
  initInput();
  initMenu();
  currentState = MENU;
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  updateInput();
  switch (currentState) {
    case BOOT:
      currentState = MENU;
      break;
    case MENU:
      updateMenu();
      drawMenu();
      break;
    case SNAKE:
      // updateSnake();
      // drawSnake();
      break;
    case PONG:
      // updatePong();
      // drawPong();
      break;
    case BREAKOUT:
      // updateBreakout();
      // drawBreakout();
      break;
    case TETRICORE:
      // updateTetricore();
      // drawTetricore();
      break;
  }
}