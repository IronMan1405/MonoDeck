#pragma once

enum AppState {
    BOOT,
    MENU,
    SNAKE,
    PONG,
    BREAKOUT,
    TETRICORE
};

extern AppState currentState;