#include <stdlib.h>
#include <stdio.h>
#include "hardware/adc.h"

#include "flappy.h"
#include "drivers/sh110x/sh110x.h"
#include "core/app_state.h"
#include "core/engine.h"
#include "core/input.h"
#include "platform/platform_time.h"
#include "platform/platform_storage.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define PIPE_SPEED 1
#define GAP_MIN 10
#define GAP_MAX 40

static bool canExit = true;

static unsigned long lastStepTime;
static unsigned long stepInterval = 10;

static int highScore = 0;
static int score;

FlappyState flappyState = FLAPPY_TITLE;

FlappyCoords bird;
FlappyCoords pipe;

const int birdSize = 4;
const int pipeWidth = 18;

float velocity = 0;
float gravity;
float flapPower;

int gapY;
int gapSize;

bool pipeScored = false;

static void handleFlappyInput();
static void updateBird();
static void updatePipe();
static void updateScore();
static void checkCollisions();
static void gameOver();

static void drawFlappyGame();
static void drawFlappyTitle();
static void drawFlappyGameOver();
static void drawFlappyPause();
static void drawFlappyExitWarning();

void initFlappy(void) {
    // srand(platform_millis());
    uint32_t seed = adc_read() ^ platform_millis();
    srand(seed);

    sh110x_clear();
    sh110x_update();

    flappyState = FLAPPY_TITLE;

    canExit = false;

    score = 0;

    loadStorage();
    highScore = gStorage.flappy_hs;

    if (highScore < 0 || highScore > 999) highScore = 0;

    lastStepTime = platform_millis();

    bird.x = 20;
    bird.y = SCREEN_HEIGHT / 2;
    velocity = 0;

    pipe.x = SCREEN_WIDTH;
    gapY = 20;
    gapSize = 32;

    gravity = 0.35;
    flapPower = 3.0;
}

void updateFlappy(void) {
    switch (flappyState) {
        case FLAPPY_TITLE:
            if (isPressed(BTN_START)) {
                flappyState = FLAPPY_PLAYING;
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
        case FLAPPY_PLAYING:
            handleFlappyInput();

            if (platform_millis() - lastStepTime >= stepInterval) {
                lastStepTime = platform_millis();
                updateBird();
                updatePipe();
                updateScore();
                checkCollisions();
            }
            if (isPressed(BTN_START)) {
                flappyState = FLAPPY_PAUSE;
            }
            break;
        case FLAPPY_PAUSE:
            if (isPressed(BTN_START)) {
                flappyState = FLAPPY_PLAYING;
            }
            if (canExit && isPressed(BTN_B)) {
                flappyState = FLAPPY_EXIT_WARN;
            }
            break;
        case FLAPPY_EXIT_WARN:
            if (!isHeld(BTN_A)) {
                canExit = true;
            }
            if (canExit && isPressed(BTN_A)) {
                canExit = false;
                requestExitToMenu();
            }
            if (isPressed(BTN_B)) {
                flappyState = FLAPPY_PAUSE;
            }
            break;
        case FLAPPY_OVER:
            if (!isHeld(BTN_B)) {
                canExit = true;
            }
            if (canExit && isPressed(BTN_B)) {
                canExit = false;
                requestExitToMenu();
            }

            if (isPressed(BTN_A)) {
                initFlappy();
                flappyState = FLAPPY_TITLE;
                lastStepTime = platform_millis();
                break;
            }
            break;
    }
}

void drawFlappy(void) {
    sh110x_clear();
    
    switch (flappyState) {
        case FLAPPY_TITLE:
            drawFlappyTitle();
            break;
        case FLAPPY_PLAYING:
            drawFlappyGame();
            break;
        case FLAPPY_PAUSE:
            drawFlappyPause();
            break;
        case FLAPPY_EXIT_WARN:
            drawFlappyExitWarning();
            break;
        case FLAPPY_OVER:
            drawFlappyGameOver();
            break;
    }
    sh110x_update();
}

static void handleFlappyInput() {
    if (isPressed(BTN_A)) {
        velocity = -flapPower;
    }
}

static void updateBird() {
    velocity += gravity;
    bird.y += velocity;
    if (bird.y < 0) bird.y = 0;
    if (bird.y > SCREEN_HEIGHT - birdSize) bird.y = SCREEN_HEIGHT - birdSize;
}

static void updatePipe() {
    pipe.x -= PIPE_SPEED;
    if (pipe.x < -pipeWidth) {
        pipe.x = SCREEN_WIDTH;
        gapY = (rand() % (GAP_MAX - GAP_MIN + 1)) + GAP_MIN;
        pipeScored = false;
    }
}

static void updateScore() {
    if (!pipeScored && pipe.x + pipeWidth < bird.x) {
        score++;
        pipeScored = true;
    }
}

static void checkCollisions() {
    if (bird.y <= 0 || bird.y >= SCREEN_HEIGHT - birdSize) {
        gameOver();
        return;
    }
    if (bird.x + birdSize >= pipe.x && bird.x <= pipe.x + pipeWidth) {
        if (bird.y <= gapY || bird.y >= gapY + gapSize) {
            gameOver();
            return;
        }
    }
}

static void gameOver() {
    flappyState = FLAPPY_OVER;
    if (score > highScore) {
        highScore = score;
        gStorage.flappy_hs = (uint16_t)highScore;
        updateStorage();
    }
}



static void drawFlappyGame() {
    sh110x_fill_rect(bird.x, bird.y, birdSize, birdSize);
    
    sh110x_fill_rect(pipe.x, 0, pipeWidth, gapY);
    sh110x_fill_rect(pipe.x, gapY + gapSize, pipeWidth, SCREEN_HEIGHT - (gapY + gapSize));
    
    char scorebuf[20];
    snprintf(scorebuf, sizeof(scorebuf), "%d", score);
    sh110x_draw_text(0, 0, scorebuf, 1);
}

static void drawFlappyTitle() {
    sh110x_draw_text(10, 10, "Flappy", 2);
    
    char highbuf[20];
    snprintf(highbuf, sizeof(highbuf), "High %d", highScore);
    sh110x_draw_text(20, 40, highbuf, 1);

    sh110x_draw_text(20, 50, "Press START", 1);
}

static void drawFlappyGameOver() {
    sh110x_draw_text(10, 10, "GAME OVER", 2);

    char buf[20];
    snprintf(buf, sizeof(buf), "Score: %d", score);
    sh110x_draw_text(20, 32, buf, 1);
    
    char highbuf[20];
    snprintf(highbuf, sizeof(highbuf), "High: %d", highScore);
    sh110x_draw_text(20, 44, highbuf, 1);

    sh110x_draw_text(5, 56, "A:Title      B:Exit", 1);
}

static void drawFlappyPause() {
    sh110x_draw_text(32, 12, "PAUSED", 2);
    sh110x_draw_text(2, 46, "START: Return to game", 1);
    sh110x_draw_text(2, 56, "B: Exit", 1);
}

static void drawFlappyExitWarning() {
    sh110x_draw_text(5, 5, "Current Progress",1);
    sh110x_draw_text(5, 15, "will be lost!", 1);
    sh110x_draw_text(2, 56, "A: Proceed  B:Cancel", 1);
}
