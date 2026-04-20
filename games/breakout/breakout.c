#include <stdlib.h>
#include <stdio.h>
#include "hardware/adc.h"

#include "breakout.h"
#include "drivers/sh110x/sh110x.h"
#include "core/app_state.h"
#include "core/engine.h"
#include "core/input.h"
#include "platform/platform_time.h"
#include "platform/platform_storage.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// #define MIN(a,b) ((a) < (b) ? (a) : (b))
// #define MAX(a,b) ((a) > (b) ? (a) : (b))

static bool canExit = true;

BreakoutStates breakoutState = BREAKOUT_TITLE;

Paddle paddle;
static int paddleWidth = 20;
static int paddleHeight = 4;
static int paddleSpeed = 5;


Ball ball;
const int ballSize = 1;
const float MAX_BALL_SPEED = 5.0;

#define BRICK_ROWS 4
#define BRICK_COLS 10

static bool bricks[BRICK_ROWS][BRICK_COLS];

const int BRICK_H = 5;
const int BRICK_W = 11;
const int BRICK_PADDING = 2;
const int BRICK_OFFSET_Y = 5;

static int highScore = 0;
static int score;
static int bricksRemaining;

static unsigned long lastBallUpdate = 0;
static const unsigned long ballInterval = 30;

static bool ballLaunched = false;

static void handleBreakoutInput();
static void updateBreakoutBall();
static void drawBreakoutGame();
static void drawBreakoutTitle();
static void drawBreakoutGameOver();
static void drawBreakoutWin();
static void drawBreakoutPause();
static void drawBreakoutExitWarning();



void initBreakout(void) {
    // srand(platform_millis());
    uint32_t seed = adc_read() ^ platform_millis();
    srand(seed);

    sh110x_clear();
    sh110x_update();

    canExit = false;
    
    breakoutState = BREAKOUT_TITLE;
    score = 0;
    
    loadStorage();
    highScore = gStorage.breakout_hs;

    if (highScore < 0 || highScore > 999) highScore = 0;

    bricksRemaining = 0;
    
    for (int i = 0; i < BRICK_ROWS; i++) {
        for (int j = 0; j < BRICK_COLS; j++) {
            bricks[i][j] = true;
            bricksRemaining++;
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

    lastBallUpdate = platform_millis();
}

void updateBreakout(void) {
    switch (breakoutState) {
        case BREAKOUT_TITLE:
            if (isPressed(BTN_START)) {
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
            if (platform_millis() - lastBallUpdate >= ballInterval) {
                lastBallUpdate = platform_millis();
                updateBreakoutBall();
            }

            if (ballLaunched && isPressed(BTN_START)) {
                breakoutState = BREAKOUT_PAUSE;
            }
            if (!ballLaunched && isPressed(BTN_A)) {
                ballLaunched = true;
                
                ball.vx = (rand() % 31 - 15) / 10.0f;
                ball.vy = -2.0f;
                
                if (ball.vx == 0) ball.vx = 0.5f;
            }
            break;
        case BREAKOUT_PAUSE:
            if (isPressed(BTN_START)) {
                lastBallUpdate = platform_millis();
                breakoutState = BREAKOUT_PLAYING;
            }
            if (canExit && isPressed(BTN_B)) {
                breakoutState = BREAKOUT_EXIT_WARN;
            }
            break;
        case BREAKOUT_EXIT_WARN:
            if (!isHeld(BTN_A)) {
                canExit = true;
            }
            if (canExit && isPressed(BTN_A)) {
                canExit = false;
                requestExitToMenu();
            }
            if (isPressed(BTN_B)) {
                breakoutState = BREAKOUT_PAUSE;
            }
            break;
        case BREAKOUT_OVER:
        case BREAKOUT_WIN:
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

void drawBreakout(void) {
    // sh110x_clear();
    // sh110x_draw_text(20, 20, "BREAKOUT", 1);
    // sh110x_draw_text(10, 40, "B: exit", 1);
    // sh110x_update();

    sh110x_clear();
    
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
        case BREAKOUT_EXIT_WARN:
            drawBreakoutExitWarning();
            break;
        case BREAKOUT_OVER:
            drawBreakoutGameOver();
            break;
        case BREAKOUT_WIN:
            drawBreakoutWin();
            break;
    }

    sh110x_update();
}



void handleBreakoutInput() {
    if (isHeld(BTN_LEFT) && paddle.x > 0) {
        paddle.x -= paddleSpeed;
    }
    if (paddle.x < 0) paddle.x = 0;
    if (isHeld(BTN_RIGHT) && paddle.x < SCREEN_WIDTH - paddleWidth) {
        paddle.x += paddleSpeed;
    }
    if (paddle.x > SCREEN_WIDTH - paddleWidth) paddle.x = SCREEN_WIDTH - paddleWidth;
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
            gStorage.breakout_hs = (uint16_t)highScore;
            updateStorage();
        }
    }
    if (ball.y + ballSize >= paddle.y && ball.x + ballSize >= paddle.x && ball.x <= paddle.x + paddleWidth) {
        ball.vy = -ball.vy;
        ball.y = paddle.y - ballSize;

        float ballCenter = (ball.x + ballSize) / 2.0f;
        float paddleCenter = (paddle.x + paddleWidth) / 2.0f;
        float hitPos = ballCenter - paddleCenter;
        
        ball.vx += hitPos * 0.05f;
    }

    for (int i = 0; i < BRICK_ROWS; i++) {
        for (int j = 0; j < BRICK_COLS; j++) {
            // if (bricks[i][j]) {
            //     int brickX = j * (BRICK_W + BRICK_PADDING);
            //     int brickY = i * (BRICK_H + BRICK_PADDING) + BRICK_OFFSET_Y;
            //     if (ball.x >= brickX && ball.x <= brickX + BRICK_W && ball.y + ballSize >= brickY && ball.y - ballSize <= brickY + BRICK_H) {
            //         ball.vy = -ball.vy;
            //         bricks[i][j] = false;
            //         int pts = (i + 1) * 10;
            //         score += pts;
            //     }
            // }

            if (!bricks[i][j]) continue;

            int brickX = j * (BRICK_W + BRICK_PADDING);
            int brickY = i * (BRICK_H + BRICK_PADDING) + BRICK_OFFSET_Y;

            if (ball.x + ballSize < brickX || ball.x - ballSize > brickX + BRICK_W) continue;
            if (ball.y + ballSize < brickY || ball.y - ballSize > brickY + BRICK_H) continue;

            float overlapLeft = (ball.x + ballSize) - brickX;
            float overlapRight = (brickX + BRICK_W) - (ball.x - ballSize);
            float overlapTop = (ball.y + ballSize) - brickY;
            float overlapBottom = (brickY + BRICK_H) - (ball.y - ballSize);

            float minOverlapX = MIN(overlapLeft, overlapRight);
            float minOverlapY = MIN(overlapTop, overlapBottom);

            if (minOverlapX < minOverlapY) { // brick hit from side
                ball.vx = -ball.vx;
            } else {
                ball.vy = -ball.vy;
            }

            bricks[i][j] = false;
            bricksRemaining--;
            score += (i+1) * 10;

            if (bricksRemaining == 0) {
                if (score > highScore) {
                    highScore = score;
                    gStorage.breakout_hs = (uint16_t)highScore;
                    updateStorage();
                }
                breakoutState = BREAKOUT_WIN;
                return;
            }
        }
    }

    if (ball.vx > MAX_BALL_SPEED) ball.vx = MAX_BALL_SPEED;
    if (ball.vx < -MAX_BALL_SPEED) ball.vx = -MAX_BALL_SPEED;
    if (ball.vy > MAX_BALL_SPEED) ball.vy = MAX_BALL_SPEED;
    if (ball.vy < -MAX_BALL_SPEED) ball.vy = -MAX_BALL_SPEED;
}




void drawBreakoutGame() {
    sh110x_fill_rect(paddle.x, paddle.y, paddleWidth, paddleHeight);
    sh110x_fill_circle(ball.x, ball.y, ballSize);

    for (int i = 0; i < BRICK_ROWS; i++) {
        for (int j = 0; j < BRICK_COLS; j++) {
            if (bricks[i][j]) {
                int brickX = j * (BRICK_W + BRICK_PADDING);
                int brickY = i * (BRICK_H + BRICK_PADDING) + BRICK_OFFSET_Y;
                sh110x_fill_rect(brickX, brickY, BRICK_W, BRICK_H);
            }
        }
    }

    if (!ballLaunched) {
        sh110x_draw_text(20, paddle.y - 12, "Press A", 1);
    }
}

void drawBreakoutTitle() {
    sh110x_draw_text(10, 10, "Breakout", 2);
    
    char highbuf[20];
    snprintf(highbuf, sizeof(highbuf), "High %d", highScore);
    sh110x_draw_text(20, 40, highbuf, 1);

    sh110x_draw_text(20, 50, "Press START", 1);
}

void drawBreakoutGameOver() {
    sh110x_draw_text(10, 10, "GAME OVER", 2);

    char buf[20];
    snprintf(buf, sizeof(buf), "Score: %d", score);
    sh110x_draw_text(20, 32, buf, 1);
    
    char highbuf[20];
    snprintf(highbuf, sizeof(highbuf), "High: %d", highScore);
    sh110x_draw_text(20, 44, highbuf, 1);

    sh110x_draw_text(5, 56, "A:Title      B:Exit", 1);
}

void drawBreakoutWin() {
    sh110x_draw_text(20, 10, "YOU WIN!", 2);
 
    char buf[20];
    snprintf(buf, sizeof(buf), "Score: %d", score);
    sh110x_draw_text(20, 32, buf, 1);
 
    char highbuf[20];
    snprintf(highbuf, sizeof(highbuf), "High: %d", highScore);
    sh110x_draw_text(20, 44, highbuf, 1);
 
    sh110x_draw_text(5, 56, "A:Title      B:Exit", 1);
}

void drawBreakoutPause() {
    sh110x_draw_text(32, 12, "PAUSED", 2);
    sh110x_draw_text(2, 46, "START: Return to game", 1);
    sh110x_draw_text(2, 56, "B: Exit", 1);
}

void drawBreakoutExitWarning() {
    sh110x_draw_text(5, 5, "Current Progress",1);
    sh110x_draw_text(5, 15, "will be lost!", 1);
    sh110x_draw_text(2, 56, "A: Proceed  B:Cancel", 1);
}