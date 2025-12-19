#include <EEPROM.h>

#include "pong.h"

#include "core/display.h"
#include "core/input.h"
#include "core/app_state.h"
#include "core/engine.h"

constexpr int SCREEN_WIDTH = 128;
constexpr int SCREEN_HEIGHT = 64;

static bool canExit = true;

enum PongStates {PONG_TITLE, PONG_PLAYING, PONG_PAUSE, PONG_OVER};
PongStates pongState = PONG_TITLE;

struct Coords {
    float x;
    float y;
};


constexpr int paddleWidth = 20;
constexpr int paddleHeight = 4;
static Coords paddle;
static int paddleSpeed = 5;

static int ballSize = 3;
static Coords ballSpeed;
static Coords ball;

constexpr int PONG_HIGHSCORE_ADDR = 4;
static int highScore = 0;
static int score = 0;

static unsigned long lastBallUpdate = 0;
const unsigned long ballInterval = 30;


static void handlePongInput();
static void updatePongBall();
static void drawPongGame();
static void drawPongTitle();
static void drawPongGameOver();
static void drawPongPause();

void initPong() {
    display.clearDisplay();
    display.display();

    EEPROM.get(PONG_HIGHSCORE_ADDR, highScore);
    if (highScore < 0 || highScore > 999) highScore = 0;

    canExit = false;


    pongState = PONG_TITLE;
    score = 0;

    paddle.x = (SCREEN_WIDTH - paddleWidth) / 2;
    paddle.y = SCREEN_HEIGHT - 10;
    
    ball.x = SCREEN_WIDTH / 2;
    ball.y = SCREEN_HEIGHT / 2;
    ballSpeed.x = 2;
    ballSpeed.y = 2;

    lastBallUpdate = millis();
}

void updatePong() {
    switch (pongState) {
        case PONG_TITLE:
            if (isPressed(BTN_A)) {
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
            if (millis() - lastBallUpdate >= ballInterval) {
                lastBallUpdate = millis();
                updatePongBall();
            }

            if (isPressed(BTN_A)) {
                pongState = PONG_PAUSE;
            }
            break;
        case PONG_PAUSE:
            if (isPressed(BTN_A)) {
                lastBallUpdate = millis();
                pongState = PONG_PLAYING;
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

void drawPong() {
    display.clearDisplay();

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
        case PONG_OVER:
            drawPongGameOver();
            break;
    }

    display.display();
}

void handlePongInput() {
    if (isHeld(BTN_LEFT) && paddle.x > 0) {
        paddle.x -= paddleSpeed;
    }
    if (isHeld(BTN_RIGHT) && paddle.x < SCREEN_WIDTH - paddleWidth) {
        paddle.x += paddleSpeed;
    }
}

void updatePongBall() {
    ball.x += ballSpeed.x;
    ball.y += ballSpeed.y;

    if (ball.x <= 0 || ball.x >= SCREEN_WIDTH - ballSize) {
        ballSpeed.x = -ballSpeed.x;
    }
    if (ball.y <= 0) {
        ballSpeed.y = -ballSpeed.y;
    }
    if (ball.y > SCREEN_HEIGHT) {
        pongState = PONG_OVER;
        if (score > highScore) {
            highScore = score;
            EEPROM.put(PONG_HIGHSCORE_ADDR, highScore);
            EEPROM.commit();
        }
    }
    if (ball.y + ballSize >= paddle.y && ball.x + ballSize >= paddle.x && ball.x <= paddle.x + paddleWidth) {
        ballSpeed.y = -ballSpeed.y;
        ball.y = paddle.y - ballSize;

        static float ballCenter = (ball.x + ballSize) / 2.0f;
        static float paddleCenter = (paddle.x + paddleWidth) / 2.0f;
        float hitPos = ballCenter - paddleCenter;
        
        ballSpeed.x += hitPos * 0.05f;

        score++;

        constexpr float MAX_BALL_SPEED_X = 4.0;
        constexpr float MAX_BALL_SPEED_Y = 4.0;
        constexpr float SPEED_INC_Y = 0.2;
        constexpr float SPEED_INC_X = 0.1;

        if (ballSpeed.y > 0) {
            ballSpeed.y = min(ballSpeed.y + SPEED_INC_Y, MAX_BALL_SPEED_X);
        } else {
            ballSpeed.y = max(ballSpeed.y - SPEED_INC_Y, -MAX_BALL_SPEED_Y);
        }
        if (ballSpeed.x > 0) {
            ballSpeed.x = min(ballSpeed.x + SPEED_INC_X, MAX_BALL_SPEED_X);
        } else {
            ballSpeed.x = max(ballSpeed.x - SPEED_INC_X, -MAX_BALL_SPEED_X);
        }
    }
}




void drawPongTitle() {
    display.setTextSize(2);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(10, 10);
    display.println("Pong");
    
    display.setTextSize(1);
    display.setCursor(20, 40);
    display.print("High: ");
    display.print(highScore);

    display.setCursor(20, 50);
    display.println("Press A to start");
}

void drawPongGame() {
    display.fillRect(paddle.x, paddle.y, paddleWidth, paddleHeight, SH110X_WHITE);
    display.fillRect(ball.x, ball.y, ballSize, ballSize, SH110X_WHITE);

    display.setTextSize(1);
    display.setCursor(2, 2);
    display.print(score);
}

void drawPongPause() {
    display.setTextSize(1);
    display.setCursor(32, 32);
    display.print("PAUSED");
}

void drawPongGameOver() {
    display.setTextSize(2);
    display.setCursor(10, 10);
    display.print("GAME OVER");
    
    display.setTextSize(1);
    display.setCursor(20, 32);
    display.print("Score: ");
    display.print(score);
    
    // display.setCursor(20, 44);
    // display.print("High: ");
    // display.print(highScore);

    display.setCursor(5, 56);
    display.print("A:Title      B:Exit");
}