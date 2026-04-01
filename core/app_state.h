#ifndef APP_STATE_H
#define APP_STATE_H

typedef enum {
    BOOT,
    MENU,
    IN_GAME
} AppState;

extern AppState currentState;

#endif