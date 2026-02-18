#ifndef APP_STATE_H
#define APP_STATE_H

typedef enum {
    BOOT,
    MENU,
    SNAKE,
    PONG,
    BREAKOUT,
    TETRICORE
} AppState;

extern AppState currentState;

#endif