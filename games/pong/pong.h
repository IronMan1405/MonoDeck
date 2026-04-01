#ifndef PONG_H
#define PONG_H

typedef enum {
    PONG_TITLE, PONG_PLAYING, PONG_PAUSE, PONG_OVER, PONG_EXIT_WARN
} PongStates;

typedef struct {
    float x;
    float y;
} Coords;

void initPong(void);
void updatePong(void);
void drawPong(void);

#endif