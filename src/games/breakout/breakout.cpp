#include <EEPROM.h>

#include "breakout.h"

#include "core/display.h"
#include "core/input.h"
#include "core/app_state.h"
#include "core/engine.h"

constexpr int SCREEN_WIDTH = 128;
constexpr int SCREEN_HEIGHT = 64;

static bool canExit = true;

enum BreakoutStates {BREAKOUT_TITLE, BREAKOUT_PLAYING, BREAKOUT_PAUSE, BREAKOUT_OVER};
BreakoutStates breakoutState = BREAKOUT_TITLE;

struct Paddle {
    int x;
    int y;
};

struct Ball {
    float x, y;
    float vx, vy;
};

Paddle paddle;
static int paddleWidth = 20;
static int paddleHeight = 4;
static int paddleSpeed = 5;


Ball ball;
constexpr int ballSize = 2;
constexpr float MAX_BALL_SPEED = 5.0;

constexpr int BRICK_ROWS = 4;
constexpr int BRICK_COLS = 8;

static bool bricks[BRICK_ROWS][BRICK_COLS];

constexpr int BRICK_H = 5;
constexpr int BRICK_W = 14;
constexpr int BRICK_PADDING = 2;
constexpr int BRICK_OFFSET_Y = 5;

constexpr int BREAKOUT_HIGHSCORE_ADDR = 8;
static int highScore = 0;
static int score;

static unsigned long lastBallUpdate = 0;
const unsigned long ballInterval = 30;

static bool ballLaunched = false;

static void handleBreakoutInput();
static void updateBreakoutBall();
static void drawBreakoutGame();
static void drawBreakoutTitle();
static void drawBreakoutGameOver();
static void drawBreakoutPause();


void initBreakout() {
    display.clearDisplay();
    display.display();

    EEPROM.get(BREAKOUT_HIGHSCORE_ADDR, highScore);
    if (highScore < 0 || highScore > 999) highScore = 0;

    canExit = false;

    breakoutState = BREAKOUT_TITLE;
    score = 0;

    for (int i = 0; i < BRICK_ROWS; i++) {
        for (int j = 0; j < BRICK_COLS; j++) {
            bricks[i][j] = true;
        }
    }

    paddle.x = (SCREEN_WIDTH - paddleWidth) / 2;
    paddle.y = SCREEN_HEIGHT - 10;

    int brickAreaBottom = BRICK_OFFSET_Y + BRICK_ROWS * (BRICK_H + BRICK_PADDING);
    ball.x = SCREEN_WIDTH / 2;
    ball.y = brickAreaBottom + 6;
    ball.vx = 2.0;
    ball.vy = -2.0;

    ballLaunched = false;

    lastBallUpdate = millis();
}

void updateBreakout() {
    switch (breakoutState) {
        case BREAKOUT_TITLE:
            if (isPressed(BTN_A)) {
                breakoutState = BREAKOUT_PLAYING;
            }
            if (!isHeld(BTN_B)) {
                canExit = true;
            }
            if (canExit && isPressed(BTN_B)) {
                canExit = false;
                requestExitToMenu();
                return;
            }
            break;
        case BREAKOUT_PLAYING:
            handleBreakoutInput();
            if (millis() - lastBallUpdate >= ballInterval) {
                lastBallUpdate = millis();
                updateBreakoutBall();
            }

            if (ballLaunched && isPressed(BTN_A)) {
                breakoutState = BREAKOUT_PAUSE;
            }
            if (!ballLaunched && isPressed(BTN_A)) {
                ballLaunched = true;
                
                ball.vx = random(-15,16) / 10.0f;
                ball.vy = -2.0f;
                
                if (ball.vx == 0) ball.vx = 0.5f;
            }
            break;
        case BREAKOUT_PAUSE:
            if (isPressed(BTN_A)) {
                lastBallUpdate = millis();
                breakoutState = BREAKOUT_PLAYING;
            }
            break;
        case BREAKOUT_OVER:
            if (!isHeld(BTN_B)) {
                canExit = true;
            }
            if (canExit && isPressed(BTN_B)) {
                canExit = false;
                requestExitToMenu();
                return;
            }

            if (isPressed(BTN_A)) {
                initBreakout();
                breakoutState = BREAKOUT_TITLE;
                break;
            }
            break;
    }
}

void drawBreakout() {
    display.clearDisplay();

    switch (breakoutState) {
        case BREAKOUT_TITLE:
            drawBreakoutTitle();
            break;
        case BREAKOUT_PLAYING:
            drawBreakoutGame();
            break;
        case BREAKOUT_PAUSE:
            drawBreakoutPause();
            break;
        case BREAKOUT_OVER:
            drawBreakoutGameOver();
            break;
    }

    display.display();
}



void handleBreakoutInput() {
    if (isHeld(BTN_LEFT) && paddle.x > 0) {
        paddle.x -= paddleSpeed;
    }
    if (isHeld(BTN_RIGHT) && paddle.x < SCREEN_WIDTH - paddleWidth) {
        paddle.x += paddleSpeed;
    }
}

void updateBreakoutBall() {
    if (!ballLaunched) {
        ball.x = paddle.x + paddleWidth / 2;
        ball.y = paddle.y - ballSize - 1;
        return;
    }

    ball.x += ball.vx;
    ball.y += ball.vy;

    if (ball.x <= 0 || ball.x >= SCREEN_WIDTH - ballSize) {
        ball.vx = -ball.vx;
    }
    if (ball.y <= ballSize) {
        ball.vy = -ball.vy;
    }
    if (ball.y >= SCREEN_HEIGHT - ballSize) {
        breakoutState = BREAKOUT_OVER;
        if (score > highScore) {
            highScore = score;
            EEPROM.put(BREAKOUT_HIGHSCORE_ADDR, highScore);
            EEPROM.commit();
        }
    }
    if (ball.y + ballSize >= paddle.y && ball.x >= paddle.x && ball.x <= paddle.x + paddleWidth) {
        ball.vy = -ball.vy;
        ball.y = paddle.y - ballSize;

        float ballCenter = (ball.x + ballSize) / 2.0f;
        float paddleCenter = (paddle.x + paddleWidth) / 2.0f;
        float hitPos = ballCenter - paddleCenter;
        
        ball.vx += hitPos * 0.05f;
    }

    for (int i = 0; i < BRICK_ROWS; i++) {
        for (int j = 0; j < BRICK_COLS; j++) {
            if (bricks[i][j]) {
                int brickX = j * (BRICK_W + BRICK_PADDING);
                int brickY = i * (BRICK_H + BRICK_PADDING) + BRICK_OFFSET_Y;
                if (ball.x >= brickX && ball.x <= brickX + BRICK_W && ball.y + ballSize >= brickY && ball.y - ballSize <= brickY + BRICK_H) {
                    ball.vy = -ball.vy;
                    bricks[i][j] = false;
                    int pts = (i + 1) * 10;
                    score += pts;
                }
            }
        }
    }

    if (ball.vx > MAX_BALL_SPEED) ball.vx = MAX_BALL_SPEED;
    if (ball.vx < -MAX_BALL_SPEED) ball.vx = -MAX_BALL_SPEED;
}

void drawBreakoutGame() {
    display.fillRect(paddle.x, paddle.y, paddleWidth, paddleHeight, SH110X_WHITE);
    display.fillCircle(ball.x, ball.y, ballSize, SH110X_WHITE);

    for (int i = 0; i < BRICK_ROWS; i++) {
        for (int j = 0; j < BRICK_COLS; j++) {
            if (bricks[i][j]) {
                int brickX = j * (BRICK_W + BRICK_PADDING);
                int brickY = i * (BRICK_H + BRICK_PADDING) + BRICK_OFFSET_Y;
                display.fillRect(brickX, brickY, BRICK_W, BRICK_H, SH110X_WHITE);
            }
        }
    }

    if (!ballLaunched) {
        display.setTextSize(1);
        display.setCursor(20, paddle.y - 12);
        display.print("Press A");
    }
}

void drawBreakoutTitle() {
    display.setTextSize(2);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(10, 10);
    display.println("Breakout");
    
    display.setTextSize(1);
    display.setCursor(20, 40);
    display.print("High: ");
    display.print(highScore);

    display.setCursor(20, 50);
    display.println("Press A to start");
}

void drawBreakoutGameOver() {
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

void drawBreakoutPause() {
    display.setTextSize(1);
    display.setCursor(32, 32);
    display.print("PAUSED");
}
