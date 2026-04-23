#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hardware/adc.h"

#include "mines.h"
#include "drivers/sh110x/sh110x.h"
#include "core/app_state.h"
#include "core/engine.h"
#include "core/input.h"
#include "platform/platform_time.h"
#include "platform/platform_storage.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define COLS 8
#define ROWS 8
#define CELL_SIZE 8

#define GRID_WIDTH COLS * CELL_SIZE
#define GRID_HEIGHT ROWS * CELL_SIZE

static bool canExit = true;

static int highScore = 0;
static int score;

MinesState minesState;

CELL cells[ROWS][COLS];
SELECTED selected;

static bool firstClick = true;
static bool playerWon = false;

static void handleMinesInput();
static void generateMines();
static void computeAdjacency();
static void reveal(int x, int y);
static void updateField();
static void checkWin();



static void drawMinesGame();
static void drawMinesTitle();
static void drawMinesGameOver();
static void drawMinesPause();
static void drawMinesExitWarning();

void initMines(void) {
    uint32_t seed = adc_read() ^ platform_millis();
    srand(seed);

    sh110x_clear();
    sh110x_update();

    minesState = MINES_TITLE;

    canExit = false;

    firstClick = true;
    playerWon = false;

    score = 0;

    loadStorage();
    highScore = gStorage.mines_hs;

    if (highScore < 0 || highScore > 99999) highScore = 0;
}

void updateMines(void) {
    switch (minesState) {
        case MINES_TITLE:
            if (isPressed(BTN_START)) {
                generateMines();
                computeAdjacency();
                selected.row = 0;
                selected.col = 0;
                firstClick = true;
                playerWon = false;
                score = 0;
                minesState = MINES_PLAYING;
            }
            if (!isHeld(BTN_B)) {
                canExit = true;
            }
            if (canExit && isPressed(BTN_B)) {
                canExit = false;
                requestExitToMenu();
            }
            break;
        case MINES_PLAYING:
            handleMinesInput();

            if (isPressed(BTN_START)) {
                minesState = MINES_PAUSE;
            }
            break;
        case MINES_PAUSE:
            if (isPressed(BTN_START)) {
                minesState = MINES_PLAYING;
            }
            if (canExit && isPressed(BTN_B)) {
                minesState = MINES_EXIT_WARN;
            }
            break;
        case MINES_EXIT_WARN:
            if (!isHeld(BTN_A)) {
                canExit = true;
            }
            if (canExit && isPressed(BTN_A)) {
                canExit = false;
                requestExitToMenu();
            }
            if (isPressed(BTN_B)) {
                minesState = MINES_PAUSE;
            }
            break;
        case MINES_OVER:
            if (playerWon && score > highScore) {
                highScore = score;
                gStorage.mines_hs = highScore;
                updateStorage();
            }    

            if (!isHeld(BTN_B)) {
                canExit = true;
            }
            if (canExit && isPressed(BTN_B)) {
                canExit = false;
                requestExitToMenu();
            }

            if (isPressed(BTN_A)) {
                initMines();
                minesState = MINES_TITLE;
                break;
            }
            break;
        case MINES_REVEAL:
            if (isPressed(BTN_A)) {
                minesState = MINES_OVER;
            }
            break;
    }
}

void drawMines(void) {
    sh110x_clear();
    
    switch (minesState) {
        case MINES_TITLE:
            drawMinesTitle();
            break;
        case MINES_PLAYING:
        case MINES_REVEAL:
            drawMinesGame();
            break;
        case MINES_PAUSE:
            drawMinesPause();
            break;
        case MINES_EXIT_WARN:
            drawMinesExitWarning();
            break;
        case MINES_OVER:
            drawMinesGameOver();
            break;
    }
    sh110x_update();
}



static void handleMinesInput() {
    if (isPressed(BTN_UP)) {
        selected.row = (selected.row - 1 + ROWS) % ROWS;
    }
    if (isPressed(BTN_DOWN)) {
        selected.row = (selected.row + 1 + ROWS) % ROWS;
    }
    if (isPressed(BTN_LEFT)) {
        selected.col = (selected.col - 1 + COLS) % COLS;
    }
    if (isPressed(BTN_RIGHT)) {
        selected.col = (selected.col + 1 + COLS) % COLS;
    }
    if (isPressed(BTN_B)) {
        CELL *selCell = &cells[selected.row][selected.col];

        if (selCell->state != OPENED) {
            selCell->state = (selCell->state == FLAGGED) ? CLOSED : FLAGGED;
        }
    }
    if (isPressed(BTN_A)) {
        if (firstClick) {
            do {
                generateMines();
                computeAdjacency();
            }
            while (cells[selected.row][selected.col].isMine || cells[selected.row][selected.col].adjacent != 0);

            firstClick = false;
        }
        
        reveal(selected.col, selected.row);
        updateField();
        checkWin();
        
    }
}

static void generateMines() {
    memset(cells, 0, sizeof(cells));
    
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            cells[y][x].state = CLOSED;
        }
    }
    
    int n = ROWS * COLS / 6;

    for (int i = 0; i < n; ) {
        int x = rand() % COLS;
        int y = rand() % ROWS;

        if (!cells[y][x].isMine) {
            cells[y][x].isMine = true;
            i++;
        }
    }
}

static void computeAdjacency() {
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {

            if (cells[y][x].isMine) continue;

            int count = 0;

            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int nx = x + dx;
                    int ny = y + dy;

                    if (nx >= 0 && nx < COLS && ny >= 0 && ny < ROWS) {
                        if (cells[ny][nx].isMine) count++;
                    }
                }
            }

            cells[y][x].adjacent = count;
        }
    }
}

static void reveal(int x, int y) {
    if (x < 0 || x >= COLS || y < 0 || y >= ROWS) return;

    CELL *c = &cells[y][x];

    if (c->state == OPENED || c->state == FLAGGED) return;

    c->state = OPENED;

    if (!c->isMine) {
        score++;
    }

    if (c->adjacent == 0 && !c->isMine) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx != 0 || dy != 0) {
                    reveal(x + dx, y + dy);
                }
            }
        }
    }
}

static void updateField() {
    CELL *c = &cells[selected.row][selected.col];
    if (c->state == OPENED && c->isMine) {
        playerWon = false;

        for (int y = 0; y < ROWS; y++) {
            for (int x = 0; x < COLS; x++) {
                if (cells[y][x].isMine) {
                    cells[y][x].state = OPENED;
                }
            }
        }
        
        minesState = MINES_REVEAL;
    }
}

static void checkWin() {
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            CELL c = cells[y][x];

            if (!c.isMine && c.state != OPENED) return;
        }
    }
    playerWon = true;
    minesState = MINES_REVEAL;
}





static void drawMinesGame() {
    for (int col = 0; col < COLS; col++) {
        sh110x_draw_line(col * CELL_SIZE, 0, col * CELL_SIZE, GRID_HEIGHT);
    }
    for (int row = 0; row < ROWS; row++) {
        sh110x_draw_line(0, row * CELL_SIZE, GRID_WIDTH, row * CELL_SIZE);
    }
    
    // numbers, selected cell, open cell

    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            int px = x * CELL_SIZE;
            int py = y * CELL_SIZE;

            CELL c = cells[y][x];

            if (c.state == FLAGGED) {
                sh110x_draw_text(px + 2, py + 2, "F", 1);
            }
            else if (c.state == OPENED) {
                if (c.isMine) {
                    sh110x_draw_text(px + 2, py + 2, "M", 1);
                } 
                else if (c.adjacent > 0) {
                    char buf[2];
                    snprintf(buf, 2, "%d", c.adjacent);
                    sh110x_draw_text(px+2, py+2, buf, 1);
                } 
                else if (!c.isMine && c.adjacent == 0) {
                    sh110x_fill_rect(px, py, CELL_SIZE, CELL_SIZE);
                }
            } 

        }
    }

    int px = selected.col * CELL_SIZE;
    int py = selected.row * CELL_SIZE;

    sh110x_draw_rect(px, py, CELL_SIZE, CELL_SIZE);

    char scorebuf[20];
    snprintf(scorebuf, sizeof(scorebuf), "%d", score);
    sh110x_draw_text(70, 10, scorebuf, 1);

    if (minesState == MINES_REVEAL) {
        sh110x_draw_text(70, 30, "BOOM!", 1);
        sh110x_draw_text(70, 50, "Press A", 1);
    }
}

static void drawMinesTitle() {
    sh110x_draw_text(10, 10, "Mines", 2);
    
    char highbuf[20];
    snprintf(highbuf, sizeof(highbuf), "High %d", highScore);
    sh110x_draw_text(20, 40, highbuf, 1);

    sh110x_draw_text(20, 50, "Press START", 1);
}

static void drawMinesGameOver() {
    if (playerWon) {
        sh110x_draw_text(10, 10, "YOU WIN", 2);
    } else {
        sh110x_draw_text(10, 10, "GAME OVER", 2);
    }

    char buf[20];
    snprintf(buf, sizeof(buf), "Score: %d", score);
    sh110x_draw_text(20, 32, buf, 1);
    
    char highbuf[20];
    snprintf(highbuf, sizeof(highbuf), "High: %d", highScore);
    sh110x_draw_text(20, 44, highbuf, 1);

    sh110x_draw_text(5, 56, "A:Title      B:Exit", 1);
}

static void drawMinesPause() {
    sh110x_draw_text(32, 12, "PAUSED", 2);
    sh110x_draw_text(2, 46, "START: Return to game", 1);
    sh110x_draw_text(2, 56, "B: Exit", 1);
}

static void drawMinesExitWarning() {
    sh110x_draw_text(5, 5, "Current Progress",1);
    sh110x_draw_text(5, 15, "will be lost!", 1);
    sh110x_draw_text(2, 56, "A: Proceed  B:Cancel", 1);
}