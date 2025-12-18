#include <EEPROM.h>

#include "snake.h"

#include "core/display.h"
#include "core/input.h"
#include "core/app_state.h"
#include "core/engine.h"


static bool canExit = true;

constexpr int CELL_SIZE = 4;
constexpr int GRID_W = 128 / CELL_SIZE;
constexpr int GRID_H = 64 / CELL_SIZE;
constexpr int MAX_SNAKE_LENGTH = 64;

struct Point {
    int x;
    int y;
};

static Point snake[MAX_SNAKE_LENGTH];
static int snakeLength;
static Point food;

enum Direction {UP, DOWN, LEFT, RIGHT};
static Direction nextDirection = RIGHT;
static Direction currentDirection = RIGHT;

static unsigned long lastStepTime;
static unsigned long stepInterval = 250;

constexpr int EEPROM_ADDR_HIGHSCORE = 0;
static int highScore = 0;
static int score;

enum SnakeState {SNAKE_PLAYING, SNAKE_OVER, SNAKE_TITLE, SNAKE_PAUSE};

SnakeState snakeState = SNAKE_PLAYING;

static void handleSnakeInput();
static void stepSnake();
static void drawSnakeGame();
static void drawSnakeTitle();
static void drawSnakeGameOver();
static void drawSnakePause();

void initSnake() {
    display.clearDisplay();
    display.display();
    
    highScore = EEPROM.read(EEPROM_ADDR_HIGHSCORE);
    if (highScore < 0 || highScore > 999) highScore = 0;


    snakeState = SNAKE_TITLE;

    canExit = false;

    currentDirection = RIGHT;
    nextDirection = RIGHT;
    
    score = 0;

    snakeLength = 3;
    snake[0] = { GRID_W / 2, GRID_H / 2};
    snake[1] = { snake[0].x - 1, snake[0].y };
    snake[2] = { snake[1].x - 1, snake[1].y };

    food = { (int)random(GRID_W), (int)random(GRID_H) };

    lastStepTime = millis();
}

void updateSnake() {
    switch (snakeState) {
        case SNAKE_TITLE:
            if (isPressed(BTN_A)) {
                snakeState = SNAKE_PLAYING;
                lastStepTime = millis();
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

            if (millis() - lastStepTime >= stepInterval) {
                lastStepTime = millis();
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
                lastStepTime = millis();
                break;
            }
            break;
    }
}

void drawSnake() {
    display.clearDisplay();
    
    switch (snakeState) {
        case SNAKE_TITLE:
            drawSnakeTitle();
            break;
        case SNAKE_PLAYING:
            drawSnakeGame();
            break;
        case SNAKE_PAUSE:
            drawSnakePause();
            break;
        case SNAKE_OVER:
            drawSnakeGameOver();
            break;
    }

    display.display();
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
            EEPROM.write(EEPROM_ADDR_HIGHSCORE, highScore);
            EEPROM.commit();
        }
        snakeState = SNAKE_OVER;
        return;
    }

    for (int i = 0; i < snakeLength; i++) {
        if (newHead.x == snake[i].x && newHead.y == snake[i].y) {
            if (score > highScore) {
                highScore = score;
                EEPROM.write(EEPROM_ADDR_HIGHSCORE, highScore);
                EEPROM.commit();
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
        food = { (int)random(GRID_W), (int)random(GRID_H) };
    }
}



void drawSnakeGame() {
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 0);
    display.print("Score: ");
    display.print(score);

    for (int i = 0; i < snakeLength; i++) {
        display.fillRect(snake[i].x * CELL_SIZE, snake[i].y * CELL_SIZE, CELL_SIZE, CELL_SIZE, SH110X_WHITE);
    }
    display.fillCircle(food.x * CELL_SIZE + CELL_SIZE / 2, food.y * CELL_SIZE + CELL_SIZE / 2, CELL_SIZE / 4, SH110X_WHITE);
}

void drawSnakeGameOver() {
    display.setTextSize(2);
    display.setCursor(10, 10);
    display.print("GAME OVER");
    
    display.setTextSize(1);
    display.setCursor(20, 32);
    display.print("Score: ");
    display.print(score);
    
    display.setCursor(20, 44);
    display.print("High: ");
    display.print(highScore);

    display.setCursor(5, 56);
    display.print("A:Title      B:Exit");
}

void drawSnakeTitle() {
    display.setTextSize(2);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(10, 10);
    display.println("Snake");
    
    display.setTextSize(1);
    display.setCursor(20, 40);
    display.print("High: ");
    display.print(highScore);

    display.setCursor(20, 50);
    display.println("Press A to start");
}

void drawSnakePause() {
    display.setTextSize(1);
    display.setCursor(32, 32);
    display.print("PAUSED");
}