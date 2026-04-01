#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "tetricore.h"
#include "drivers/sh110x/sh110x.h"
#include "core/app_state.h"
#include "core/engine.h"
#include "core/input.h"
#include "platform/platform_time.h"
#include "platform/platform_storage.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

static bool canExit = true;

TetricoreStates tetricoreState = TETRICORE_TITLE;

#define CELL_SIZE 4
#define GRID_W (128 / CELL_SIZE)
#define GRID_H (64 / CELL_SIZE)

#define FIELD_W 10
#define FIELD_X ((GRID_W - FIELD_W) / 2) //- 8

#define INFO_X (FIELD_X + FIELD_W) * CELL_SIZE + 6

#define PREVIEW_X INFO_X
#define PREVIEW_Y 26

#define SHAPE_SIZE 4

uint8_t grid[GRID_H][GRID_W];
// uint8_t grid[GRID_H][FIELD_W];

Piece active;

Piece next;

const uint8_t tetrominoes[7][SHAPE_SIZE][SHAPE_SIZE] = {
    {
        {0,0,0,0},
        {1,1,1,1},
        {0,0,0,0},
        {0,0,0,0}
    },
    {
        {0,1,1,0},
        {0,1,1,0},
        {0,0,0,0},
        {0,0,0,0}
    },
    {
        {0,1,0,0},
        {1,1,1,0},
        {0,0,0,0},
        {0,0,0,0}
    },
    {
        {0,1,1,0},
        {1,1,0,0},
        {0,0,0,0},
        {0,0,0,0}
    },
    {
        {1,1,0,0},
        {0,1,1,0},
        {0,0,0,0},
        {0,0,0,0}
    },
    {
        {1,0,0,0},
        {1,1,1,0},
        {0,0,0,0},
        {0,0,0,0}
    },
    {
        {0,0,1,0},
        {1,1,1,0},
        {0,0,0,0},
        {0,0,0,0}
    }
};

unsigned long lastFall = 0;
const unsigned long FALL_INTERVAL = 300;
const unsigned long SOFT_DROP_INTERVAL = 50;

static bool softDrop = false;

unsigned long lastInput = 0;
const unsigned long INPUT_DELAY = 120;

const int TETRICORE_HIGHSCORE_ADDR = 12;
static int highScore = 0;
static int score;

ClearAnimState clearState = CLEAR_NONE;

int clearRow = -1;
unsigned long clearStart = 0;
int pendingClearCount = 0;

static void handleTetricoreInput();
static void drawTetricoreGame();
static void drawTetricoreTitle();
static void drawTetricoreGameOver();
static void drawTetricorePause();
static void drawTetricoreExitWarning();

static void spawnPiece();
static void tryMoveDown();
static void lockPiece();
static void moveLeft();
static void moveRight();
static void clearLines();
static void actuallyClearRow(int y);
static void tryRotate();
static void applyTetrisScore(int lines);
static bool canMove(int nx, int ny);
static bool isRowFull(int y);
static bool getCell(Shapes type, int rot, int x, int y);


void initTetricore(void) {
    srand(platform_millis());
    
    sh110x_clear();
    sh110x_update();

    if (highScore < 0 || highScore > 99999) highScore = 0;

    canExit = false;

    tetricoreState = TETRICORE_TITLE;
    score = 0;

    loadStorage();
    highScore = gStorage.tetricore_hs;

    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            grid[y][x] = 0;
        }
    }

    // next.type = (Shapes)random(0, 7);
    next.type = (Shapes) rand() % 7;
    next.rotation = 0;
}

void updateTetricore(void) {
    switch (tetricoreState) {
        case TETRICORE_TITLE: {
            if (isPressed(BTN_START)) {
                tetricoreState = TETRICORE_PLAYING;
                spawnPiece();
                lastFall = platform_millis();
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
        }
        case TETRICORE_PLAYING: {
            handleTetricoreInput();
            
            unsigned long interval = softDrop ? SOFT_DROP_INTERVAL : FALL_INTERVAL;
            if (platform_millis() - lastFall >= interval) {
                lastFall = platform_millis();
                tryMoveDown();

                if (softDrop) {
                    score += 1;
                }
            }
            if (isPressed(BTN_START)) {
                tetricoreState = TETRICORE_PAUSE;
            }
            break;
        }
        case TETRICORE_PAUSE:
            if (isPressed(BTN_START)) {
                lastFall = platform_millis();
                tetricoreState = TETRICORE_PLAYING;
            }
            if (canExit && isPressed(BTN_B)) {
                tetricoreState = TETRICORE_WARN_EXIT;
            }
            break;
        case TETRICORE_WARN_EXIT:
            if (!isHeld(BTN_A)) {
                canExit = true;
            }
            if (canExit && isPressed(BTN_A)) {
                canExit = false;
                requestExitToMenu();
            }
            if (isPressed(BTN_B)) {
                tetricoreState = TETRICORE_PAUSE;
            }
            break;
        case TETRICORE_OVER: {
            if (!isHeld(BTN_B)) {
                canExit = true;
            }

            if (canExit && isPressed(BTN_B)) {
                canExit = false;
                requestExitToMenu();
                return;
            }
            if (isPressed(BTN_A)) {
                initTetricore();
                tetricoreState = TETRICORE_TITLE;
                break;
            }
            break;
        }
    }

    if (clearState == CLEAR_FLASH) {
        if (platform_millis() - clearStart > 120) {
            clearState = CLEAR_NONE;
            actuallyClearRow(clearRow);
            clearRow = -1;

            clearLines();

            if (clearState == CLEAR_NONE && pendingClearCount > 0) {
                applyTetrisScore(pendingClearCount);
                pendingClearCount = 0;
            }
        }
        return;
    }
}

void drawTetricore(void) {
    // sh110x_clear();
    // sh110x_draw_text(20, 20, "TETRICORE", 1);
    // sh110x_draw_text(10, 40, "B: exit", 1);
    // sh110x_update();

    sh110x_clear();

    switch (tetricoreState) {
        case TETRICORE_TITLE:
            drawTetricoreTitle();
            break;
        case TETRICORE_PLAYING:
            drawTetricoreGame();
            break;
        case TETRICORE_PAUSE:
            drawTetricorePause();
            break;
        case TETRICORE_WARN_EXIT:
            drawTetricoreExitWarning();
            break;
        case TETRICORE_OVER:
            drawTetricoreGameOver();
            break;
    }

    sh110x_update();
}




void handleTetricoreInput() {
    if (platform_millis() - lastInput < INPUT_DELAY) return;

    softDrop = isHeld(BTN_DOWN);

    if (isHeld(BTN_LEFT)) {
        moveLeft();
        lastInput = platform_millis();
    }
    if(isHeld(BTN_RIGHT)) {
        moveRight();
        lastInput = platform_millis();
    }

    if (isPressed(BTN_B)) {
        tryRotate();
    }
}

void tryMoveDown() {
    if (canMove(active.x, active.y + 1)) {
        active.y++;
    } else {
        lockPiece();
        clearLines();
        spawnPiece();
    }
}

void lockPiece() {
    for (int sy = 0; sy < SHAPE_SIZE; sy++) {
        for (int sx = 0; sx < SHAPE_SIZE; sx++) {
        if(getCell(active.type, active.rotation, sx, sy)) {
            int gx = active.x + sx;
            int gy = active.y + sy;
            if (gy >= 0 && gy < GRID_H && gx >= 0 && gx < GRID_W) {
                grid[gy][gx] = 1;
            }
        }
        }
    }
}

void moveLeft() {
    if (canMove(active.x-1, active.y)) {
        active.x--;
    }
}

void moveRight() {
    if (canMove(active.x+1, active.y)) {
        active.x++;
    }
}

void clearLines() {
    pendingClearCount = 0;

    for (int y = GRID_H - 1; y >= 0; y--) {
        if (isRowFull(y)) {
            clearRow = y;
            clearState = CLEAR_FLASH;
            clearStart = platform_millis();
            pendingClearCount++;
            return;
        }
    }
}

void actuallyClearRow(int y) {
    for (int ty = y; ty > 0; ty--) {
        for (int x = FIELD_X; x < FIELD_X + FIELD_W; x++) {
        grid[ty][x] = grid[ty-1][x];
        }
    }
    for (int x = FIELD_X; x < FIELD_X + FIELD_W; x++) {
        grid[0][x] = 0;
    }
}

void applyTetrisScore(int lines) {
    switch (lines) {
        case 1: score += 100; break;
        case 2: score += 300; break;
        case 3: score += 500; break;
        case 4: score += 800; break;
    }
}

bool canMove(int nx, int ny) {
    for (int sy = 0; sy < SHAPE_SIZE; sy++) {
        for (int sx = 0; sx < SHAPE_SIZE; sx++) {
            if (!getCell(active.type, active.rotation, sx, sy)) continue;

            int gx = nx + sx;
            int gy = ny + sy;

            if (gx < FIELD_X || gx >= FIELD_X + FIELD_W) return false;
            if (gy < 0 || gy >= GRID_H) return false;
            if (grid[gy][gx]) return false;
        }
    }
    return true;
}

bool isRowFull(int y) {
    for (int x = FIELD_X; x < FIELD_X + FIELD_W; x++) {
        if (grid[y][x] == 0) return false; 
    }
    return true;
}

void spawnPiece() {
    active = next;
    
    active.x = FIELD_X + (FIELD_W / 2) - 2; // -2 for centering 4x4 matrix
    active.y = 0;

    // next.type = (Shapes)random(0,7);
    next.type = (Shapes) rand() % 7;
    next.rotation = 0;

    if (!canMove(active.x, active.y)) {
        tetricoreState = TETRICORE_OVER;
        if (score > highScore) {
            highScore = score;
            gStorage.tetricore_hs = (uint16_t)highScore;
            updateStorage();
        }
    }
}



bool getCell(Shapes type, int rot, int x, int y) {
    switch (rot % 4) {
        case 0: return tetrominoes[type][y][x];
        case 1: return tetrominoes[type][3 - x][y];
        case 2: return tetrominoes[type][3 - y][3 - x];
        case 3: return tetrominoes[type][x][3 - y];
    }
    return false;
}

bool canMoveRot(int nx, int ny, int rot) {
    for (int sy = 0; sy < SHAPE_SIZE; sy++) {
        for (int sx = 0; sx < SHAPE_SIZE; sx++) {
            if (!getCell(active.type, rot, sx, sy)) continue;

            int gx = nx + sx;
            int gy = ny + sy;

            if (gx < FIELD_X || gx >= FIELD_X + FIELD_W) return false;
            if (gy < 0 || gy >= GRID_H) return false;
            if (grid[gy][gx]) return false;
        }
    }
    return true;
}

void tryRotate() {
    int newRot = (active.rotation + 1) % 4;
    if (canMoveRot(active.x, active.y, newRot)) {
        active.rotation = newRot;
    }
}






void drawBlock(int x, int y) {
    int px = x * CELL_SIZE;
    int py = y * CELL_SIZE;

    sh110x_draw_rect(px, py, CELL_SIZE, CELL_SIZE);
    // display.fillRect(px + 2, py + 2, CELL_SIZE - 4, CELL_SIZE - 4, SH110X_WHITE);
}

void drawTetricoreGame() {
    bool flashOn = ((platform_millis() / 100) % 2) == 0;

    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
        if (grid[y][x]) {
            if (clearState == CLEAR_FLASH && y == clearRow) {
                if (flashOn) {
                    drawBlock(x, y);
                }
            } else {
                drawBlock(x, y);
            }
        }
        }
    }

    for (int sy = 0; sy < SHAPE_SIZE; sy++) {
        for (int sx = 0; sx < SHAPE_SIZE; sx++) {
            if (getCell(active.type, active.rotation, sx, sy)) {
                drawBlock(active.x + sx, active.y + sy);
            }
        }
    }

    int left = (FIELD_X * CELL_SIZE) - 2;
    int right = ((FIELD_X + FIELD_W) * CELL_SIZE) + 1;

    sh110x_draw_line(left, 0, left, SCREEN_HEIGHT);
    sh110x_draw_line(right, 0, right, SCREEN_HEIGHT);

    int uiX = (FIELD_X + FIELD_W) * CELL_SIZE + 8; // right of field
    int uiY = 4;

    char scorebuf[20];
    snprintf(scorebuf, sizeof(scorebuf), "SCORE %d", score);
    sh110x_draw_text(uiX, uiY, scorebuf, 1);

    char hibuf[20];
    snprintf(hibuf, sizeof(hibuf), "HI %d", highScore);
    sh110x_draw_text(uiX, uiY + 10, hibuf, 1);


    for (int sy = 0; sy < SHAPE_SIZE; sy++) {
        for (int sx = 0; sx < SHAPE_SIZE; sx++) {
            if (getCell(next.type, next.rotation, sx, sy)) {
                int px = PREVIEW_X + sx * CELL_SIZE;
                int py = PREVIEW_Y + sy * CELL_SIZE;

                sh110x_draw_rect(px, py, CELL_SIZE, CELL_SIZE);
                sh110x_fill_rect(px + 2, py + 2, CELL_SIZE - 4, CELL_SIZE - 4);
            }
        }
    }

    int boxW = SHAPE_SIZE * CELL_SIZE + 4;
    int boxH = SHAPE_SIZE * CELL_SIZE + 4;

    sh110x_draw_rect(PREVIEW_X - 2, PREVIEW_Y - 2, boxW, boxH);
}

void drawTetricoreTitle() {
    sh110x_draw_text(10, 10, "Tetricore", 2);
    
    char highbuf[20];
    snprintf(highbuf, sizeof(highbuf), "High %d", highScore);
    sh110x_draw_text(20, 40, highbuf, 1);

    sh110x_draw_text(20, 50, "Press START", 1);
}

void drawTetricoreGameOver() {
    sh110x_draw_text(10, 10, "GAME OVER", 2);

    char buf[20];
    snprintf(buf, sizeof(buf), "Score: %d", score);
    sh110x_draw_text(20, 32, buf, 1);
    
    char highbuf[20];
    snprintf(highbuf, sizeof(highbuf), "High: %d", highScore);
    sh110x_draw_text(20, 44, highbuf, 1);

    sh110x_draw_text(5, 56, "A:Title      B:Exit", 1);
}


void drawTetricorePause() {
    sh110x_draw_text(32, 12, "PAUSED", 2);
    sh110x_draw_text(2, 46, "START: Return to game", 1);
    sh110x_draw_text(2, 56, "B: Exit", 1);
}

void drawTetricoreExitWarning() {
    sh110x_draw_text(5, 5, "Current Progress",1);
    sh110x_draw_text(5, 15, "will be lost!", 1);
    sh110x_draw_text(2, 56, "A: Proceed  B:Cancel", 1);
}