#ifndef SNAKE_H
#define SNAKE_H

typedef struct {
    int x;
    int y;
} Point;

typedef enum {UP, DOWN, LEFT, RIGHT} Direction;

typedef enum {SNAKE_PLAYING, SNAKE_OVER, SNAKE_TITLE, SNAKE_PAUSE, SNAKE_EXIT_WARN} SnakeState;

void initSnake(void);
void updateSnake(void);
void drawSnake(void);

#endif