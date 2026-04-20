#ifndef FLAPPY_H
#define FLAPPY_H

typedef struct {
  int x;
  float y;
} FlappyCoords;

typedef enum {FLAPPY_PLAYING, FLAPPY_OVER, FLAPPY_TITLE, FLAPPY_PAUSE, FLAPPY_EXIT_WARN} FlappyState;

void initFlappy(void);
void updateFlappy(void);
void drawFlappy(void);

#endif