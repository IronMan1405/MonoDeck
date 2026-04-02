#include <stdlib.h>
#include <stdio.h>

#include "pong.h"
#include "drivers/sh110x/sh110x.h"
#include "core/app_state.h"
#include "core/engine.h"
#include "core/input.h"
#include "platform/platform_time.h"
#include "platform/platform_storage.h"

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

static bool canExit = true;

PongStates pongState = PONG_TITLE;

const int paddleWidth = 20;
const int paddleHeight = 4;
static Coords paddle;
static int paddleSpeed = 5;

const static int ballSize = 3;
static Coords ballSpeed;
static Coords ball;

static int highScore = 0;
static int score = 0;

static unsigned long lastBallUpdate = 0;
static const unsigned long ballInterval = 30;


static void handlePongInput();
static void updatePongBall();
static void drawPongGame();
static void drawPongTitle();
static void drawPongGameOver();
static void drawPongPause();
static void drawPongExitWarning();


void initPong(void) {
    sh110x_clear();
    sh110x_update();

    pongState = PONG_TITLE;

    canExit = false;

    score = 0;

    loadStorage();
    highScore = gStorage.pong_hs;

    if (highScore < 0 || highScore > 999) highScore = 0;

    paddle.x = (SCREEN_WIDTH - paddleWidth) / 2;
    paddle.y = SCREEN_HEIGHT - 10;
    
    ball.x = SCREEN_WIDTH / 2;
    ball.y = SCREEN_HEIGHT / 2;
    ballSpeed.x = 2;
    ballSpeed.y = 2;

    lastBallUpdate = platform_millis();
}

void updatePong(void) {
    // if (isPressed(BTN_B)) {
    //     requestExitToMenu();
    // }

    switch (pongState) {
        case PONG_TITLE:
            if (isPressed(BTN_START)) {
                pongState = PONG_PLAYING;
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
        case PONG_PLAYING:
            handlePongInput();
            if (platform_millis() - lastBallUpdate >= ballInterval) {
                lastBallUpdate = platform_millis();
                updatePongBall();
            }

            if (isPressed(BTN_START)) {
                pongState = PONG_PAUSE;
            }
            break;
        case PONG_PAUSE:
            if (isPressed(BTN_START)) {
                lastBallUpdate = platform_millis();
                pongState = PONG_PLAYING;
            }
            if (canExit && isPressed(BTN_B)) {
                pongState = PONG_EXIT_WARN;
            }
            break;
        case PONG_EXIT_WARN:
            if (!isHeld(BTN_A)) {
                canExit = true;
            }
            if (canExit && isPressed(BTN_A)) {
                canExit = false;
                requestExitToMenu();
            }
            if (isPressed(BTN_B)) {
                pongState = PONG_PAUSE;
            }
            break;
        case PONG_OVER:
            if (!isHeld(BTN_B)) {
                canExit = true;
            }
            if (canExit && isPressed(BTN_B)) {
                canExit = false;
                requestExitToMenu();
                return;
            }

            if (isPressed(BTN_A)) {
                initPong();
                pongState = PONG_TITLE;
                break;
            }
            break;
    }
}

void drawPong(void) {
    // sh110x_clear();
    // sh110x_draw_text(20, 20, "PONG", 1);
    // sh110x_draw_text(10, 40, "B: exit", 1);
    // sh110x_update();
    sh110x_clear();

    switch (pongState) {
        case PONG_TITLE:
            drawPongTitle();
            break;
        case PONG_PLAYING:
            drawPongGame();
            break;
        case PONG_PAUSE:
            drawPongPause();
            break;
        case PONG_EXIT_WARN:
            drawPongExitWarning();
            break;
        case PONG_OVER:
            drawPongGameOver();
            break;
    }

    sh110x_update();
}




static void handlePongInput() {
    if (isHeld(BTN_LEFT) && paddle.x > 0) {
        paddle.x -= paddleSpeed;
    }
    if (isHeld(BTN_RIGHT) && paddle.x < SCREEN_WIDTH - paddleWidth) {
        paddle.x += paddleSpeed;
    }
}

static void updatePongBall() {
    ball.x += ballSpeed.x;
    ball.y += ballSpeed.y;

    if (ball.x <= 0 || ball.x >= SCREEN_WIDTH - ballSize) {
        ballSpeed.x = -ballSpeed.x;
    }
    if (ball.y <= ballSize) {
        ballSpeed.y = -ballSpeed.y;
    }
    if (ball.y > SCREEN_HEIGHT) {
        pongState = PONG_OVER;
        if (score > highScore) {
            highScore = score;
            gStorage.pong_hs = (uint16_t)highScore;
            updateStorage();
        }
    }
    if (ball.y + ballSize >= paddle.y && ball.x + ballSize >= paddle.x && ball.x <= paddle.x + paddleWidth) {
        ballSpeed.y = -ballSpeed.y;
        ball.y = paddle.y - ballSize;

        float ballCenter = (ball.x + ballSize) / 2.0f;
        float paddleCenter = (paddle.x + paddleWidth) / 2.0f;
        float hitPos = ballCenter - paddleCenter;
        
        ballSpeed.x += hitPos * 0.05f;

        score++;

        const float MAX_BALL_SPEED_X = 4.0;
        const float MAX_BALL_SPEED_Y = 4.0;
        const float SPEED_INC_Y = 0.2;
        const float SPEED_INC_X = 0.1;

        if (ballSpeed.y > 0) {
            ballSpeed.y = MIN(ballSpeed.y + SPEED_INC_Y, MAX_BALL_SPEED_X);
        } else {
            ballSpeed.y = MAX(ballSpeed.y - SPEED_INC_Y, -MAX_BALL_SPEED_Y);
        }
        if (ballSpeed.x > 0) {
            ballSpeed.x = MIN(ballSpeed.x + SPEED_INC_X, MAX_BALL_SPEED_X);
        } else {
            ballSpeed.x = MAX(ballSpeed.x - SPEED_INC_X, -MAX_BALL_SPEED_X);
        }
    }
}



void drawPongTitle() {
    sh110x_draw_text(10, 10, "Pong", 2);
    
    char highbuf[20];
    snprintf(highbuf, sizeof(highbuf), "High %d", highScore);
    sh110x_draw_text(20, 40, highbuf, 1);

    sh110x_draw_text(20, 50, "Press START", 1);
}

void drawPongGame() {
    sh110x_fill_rect(paddle.x, paddle.y, paddleWidth, paddleHeight);
    sh110x_fill_rect(ball.x, ball.y, ballSize, ballSize);

    char buf[20];
    snprintf(buf, sizeof(buf), "Score: %d", score);
    sh110x_draw_text(0, 0, buf, 1);
}

void drawPongPause() {
    sh110x_draw_text(32, 12, "PAUSED", 2);
    sh110x_draw_text(2, 46, "START: Return to game", 1);
    sh110x_draw_text(2, 56, "B: Exit", 1);
}

void drawPongExitWarning() {
    sh110x_draw_text(5, 5, "Current Progress",1);
    sh110x_draw_text(5, 15, "will be lost!", 1);
    sh110x_draw_text(2, 56, "A: Proceed  B:Cancel", 1);
}

void drawPongGameOver() {
    sh110x_draw_text(10, 10, "GAME OVER", 2);

    char buf[20];
    snprintf(buf, sizeof(buf), "Score: %d", score);
    sh110x_draw_text(20, 32, buf, 1);
    
    char highbuf[20];
    snprintf(highbuf, sizeof(highbuf), "High: %d", highScore);
    sh110x_draw_text(20, 44, highbuf, 1);

    sh110x_draw_text(5, 56, "A:Title      B:Exit", 1);
}