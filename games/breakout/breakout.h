#ifndef BREAKOUT_H
#define BREAKOUT_H

typedef enum {BREAKOUT_TITLE, BREAKOUT_PLAYING, BREAKOUT_PAUSE, BREAKOUT_OVER} BreakoutStates;

typedef struct {
    int x;
    int y;
} Paddle;

typedef struct {
    float x, y;
    float vx, vy;
} Ball;


void initBreakout(void);
void updateBreakout(void);
void drawBreakout(void);

#endif