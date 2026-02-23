#include <stdlib.h>
#include <stdio.h>

#include "snake.h"
#include "drivers/sh110x/sh110x.h"
#include "core/app_state.h"
#include "core/engine.h"
#include "core/input.h"
#include "platform/platform_time.h"

static bool canExit = true;

#define CELL_SIZE 4
#define GRID_W 128 / CELL_SIZE
#define GRID_H 64 / CELL_SIZE
#define MAX_SNAKE_LENGTH 64

static Point snake[MAX_SNAKE_LENGTH];
static int snakeLength;
static Point food;

static Direction nextDirection = RIGHT;
static Direction currentDirection = RIGHT;

static unsigned long lastStepTime;
static unsigned long stepInterval = 250;

const int EEPROM_ADDR_HIGHSCORE = 0;
static int highScore = 0;
static int score;

SnakeState snakeState = SNAKE_TITLE;

static void handleSnakeInput();
static void stepSnake();
static void drawSnakeGame();
static void drawSnakeTitle();
static void drawSnakeGameOver();
static void drawSnakePause();

void initSnake(void) {
    srand(platform_millis());

    sh110x_clear();
    sh110x_update();

    snakeState = SNAKE_TITLE;

    canExit = false;

    currentDirection = RIGHT;
    nextDirection = RIGHT;
    
    score = 0;

    snakeLength = 3;
    snake[0] = (Point){GRID_W / 2, GRID_H / 2};
    snake[1] = (Point){snake[0].x - 1, snake[0].y};
    snake[2] = (Point){snake[1].x - 1, snake[1].y};

    food = (Point){rand() % GRID_W, rand() % GRID_H};

    lastStepTime = platform_millis();
}

void updateSnake(void) {
    // if (isPressed(BTN_B)) {
    //     requestExitToMenu();
    // }

    switch (snakeState) {
        case SNAKE_TITLE:
            if (isPressed(BTN_A)) {
                snakeState = SNAKE_PLAYING;
                lastStepTime = platform_millis();
            }
            if (!isHeld(BTN_B)) {
                canExit = true;
            }
            if (canExit && isPressed(BTN_B)) {
                canExit = false;
                requestExitToMenu();
            }
            break;
        case SNAKE_PLAYING:
            handleSnakeInput();

            if (platform_millis() - lastStepTime >= stepInterval) {
                lastStepTime = platform_millis();
                stepSnake();
            }
            if (isPressed(BTN_A)) {
                snakeState = SNAKE_PAUSE;
            }
            break;
        case SNAKE_PAUSE:
            if (isPressed(BTN_A)) {
                snakeState = SNAKE_PLAYING;
            }
            break;
        case SNAKE_OVER:
            if (!isHeld(BTN_B)) {
                canExit = true;
            }
            if (canExit && isPressed(BTN_B)) {
                canExit = false;
                requestExitToMenu();
            }

            if (isPressed(BTN_A)) {
                initSnake();
                snakeState = SNAKE_TITLE;
                lastStepTime = platform_millis();
                break;
            }
            break;
    }
}

void drawSnake(void) {
    sh110x_clear();
    // sh110x_draw_text(20, 20, "Snake", 1);
    // sh110x_draw_text(10, 40, "B: exit", 1);
    
    switch (snakeState) {
        case SNAKE_TITLE:
            drawSnakeTitle();
            // sh110x_update();
            break;
        case SNAKE_PLAYING:
            drawSnakeGame();
            // sh110x_update();
            break;
        case SNAKE_PAUSE:
            drawSnakePause();
            // sh110x_update();
            break;
        case SNAKE_OVER:
            drawSnakeGameOver();
            // sh110x_update();
            break;
    }
    sh110x_update();
}




void handleSnakeInput() {
    if (isPressed(BTN_UP) && currentDirection != DOWN)  nextDirection = UP;
    if (isPressed(BTN_DOWN) && currentDirection != UP)  nextDirection = DOWN;
    if (isPressed(BTN_LEFT) && currentDirection != RIGHT) nextDirection = LEFT;
    if (isPressed(BTN_RIGHT) && currentDirection != LEFT) nextDirection = RIGHT;
}

void stepSnake() {
    Point newHead = snake[0];

    currentDirection = nextDirection;
    switch (currentDirection) {
        case UP:
            newHead.y -= 1;
            break;
        case DOWN:
            newHead.y += 1;
            break;
        case LEFT:
            newHead.x -= 1;
            break;
        case RIGHT:
            newHead.x += 1;
            break;
    }

    if (newHead.x < 0 || newHead.x >= GRID_W || newHead.y < 0 || newHead.y >= GRID_H) {
        if (score > highScore) {
            highScore = score;
        }
        snakeState = SNAKE_OVER;
        return;
    }

    for (int i = 0; i < snakeLength; i++) {
        if (newHead.x == snake[i].x && newHead.y == snake[i].y) {
            if (score > highScore) {
                highScore = score;
            }
            snakeState = SNAKE_OVER;
            return;
        }
    }

    bool ateFood = (newHead.x == food.x && newHead.y == food.y);
    if (ateFood && snakeLength < MAX_SNAKE_LENGTH) {
        snakeLength++;
    }
    
    for (int i = snakeLength - 1; i > 0; i--) {
        snake[i] = snake[i - 1];
    }
    snake[0] = newHead;

    if (ateFood) {
        score++;
        food = (Point){ rand() % GRID_W, rand() % GRID_H };
    }
}




void drawSnakeGame() {

    char buf[20];
    snprintf(buf, sizeof(buf), "Score: %d", score);
    sh110x_draw_text(0, 0, buf, 1);

    for (int i = 0; i < snakeLength; i++) {
        sh110x_fill_rect(snake[i].x * CELL_SIZE, snake[i].y * CELL_SIZE, CELL_SIZE, CELL_SIZE);
    }
    sh110x_fill_circle(food.x * CELL_SIZE + CELL_SIZE / 2, food.y * CELL_SIZE + CELL_SIZE / 2, CELL_SIZE / 4);
}

void drawSnakeGameOver() {
    sh110x_draw_text(10, 10, "GAME OVER", 2);

    char buf[20];
    snprintf(buf, sizeof(buf), "Score: %d", score);
    sh110x_draw_text(20, 32, buf, 1);
    
    char highbuf[20];
    snprintf(highbuf, sizeof(highbuf), "High: %d", highScore);
    sh110x_draw_text(20, 44, highbuf, 1);

    sh110x_draw_text(5, 56, "A:Title      B:Exit", 1);
}

void drawSnakeTitle() {
    sh110x_draw_text(10, 10, "Snake", 2);
    
    char highbuf[20];
    snprintf(highbuf, sizeof(highbuf), "High %d", highScore);
    sh110x_draw_text(20, 40, highbuf, 1);

    sh110x_draw_text(20, 50, "Press A to start", 1);
}

void drawSnakePause() {
    sh110x_draw_text(32, 32, "PAUSED", 1);
}