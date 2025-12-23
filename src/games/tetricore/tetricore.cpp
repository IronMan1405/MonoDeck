#include <EEPROM.h>

#include "tetricore.h"

#include "core/display.h"
#include "core/input.h"
#include "core/app_state.h"
#include "core/engine.h"

constexpr int SCREEN_WIDTH = 128;
constexpr int SCREEN_HEIGHT = 64;

static bool canExit = true;

enum TetricoreStates {TETRICORE_TITLE, TETRICORE_PLAYING, TETRICORE_PAUSE, TETRICORE_OVER};
TetricoreStates tetricoreState = TETRICORE_TITLE;

constexpr int CELL_SIZE = 4;
constexpr int GRID_W = (128 / CELL_SIZE);
constexpr int GRID_H = (64 / CELL_SIZE);

constexpr int FIELD_W = 10;
constexpr int FIELD_X = ((GRID_W - FIELD_W) / 2) - 8;

constexpr int INFO_X = (FIELD_X + FIELD_W) * CELL_SIZE + 6;

constexpr int PREVIEW_X = INFO_X;
constexpr int PREVIEW_Y = 26;

constexpr int SHAPE_SIZE = 4;

uint8_t grid[GRID_H][GRID_W];

enum Shapes {I, O, T, S, Z, J, L};

struct Piece {
    int x;
    int y;
    Shapes type;
    uint8_t rotation;
};

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
constexpr unsigned long FALL_INTERVAL = 300;
constexpr unsigned long SOFT_DROP_INTERVAL = 50;

static bool softDrop = false;

unsigned long lastInput = 0;
constexpr unsigned long INPUT_DELAY = 120;

constexpr int TETRICORE_HIGHSCORE_ADDR = 12;
static int highScore = 0;
static int score;

enum ClearAnimState { CLEAR_NONE, CLEAR_FLASH };
ClearAnimState clearState = CLEAR_NONE;

int clearRow = -1;
unsigned long clearStart = 0;

static void handleTetricoreInput();
static void drawTetricoreGame();
static void drawTetricoreTitle();
static void drawTetricoreGameOver();
static void drawTetricorePause();

static void spawnPiece();
static void tryMoveDown();
static void lockPiece();
static void moveLeft();
static void moveRight();
static void clearLines();
static void actuallyClearRow(int y);
static void tryRotate();
static bool canMove(int nx, int ny);
static bool isRowFull(int y);
static bool getCell(Shapes type, int rot, int x, int y);

void initTetricore() {
    canExit = false;
    display.clearDisplay();
    display.display();

    EEPROM.get(TETRICORE_HIGHSCORE_ADDR, highScore);
    if (highScore < 0 || highScore > 99999) highScore = 0;

    canExit = false;

    tetricoreState = TETRICORE_TITLE;
    score = 0;

    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            grid[y][x] = 0;
        }
    }

    next.type = (Shapes)random(0, 7);
    next.rotation = 0;
}

void updateTetricore() {
    switch (tetricoreState) {
        case TETRICORE_TITLE: {
            if (isPressed(BTN_A)) {
                tetricoreState = TETRICORE_PLAYING;
                spawnPiece();
                lastFall = millis();
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
            if (millis() - lastFall >= interval) {
                lastFall = millis();
                tryMoveDown();

                if (softDrop) {
                    score += 1;
                }
            }
            if (isPressed(BTN_A)) {
                tetricoreState = TETRICORE_PAUSE;
            }
            break;
        }
        case TETRICORE_PAUSE: {
            if (isPressed(BTN_A)) {
                lastFall = millis();
                tetricoreState = TETRICORE_PLAYING;
            }
            break;
        }
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
        if (millis() - clearStart > 120) {
            clearState = CLEAR_NONE;
            actuallyClearRow(clearRow);
            clearRow = -1;
        }
        return;
    }
}

void drawTetricore() {
    display.clearDisplay();

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
        case TETRICORE_OVER:
            drawTetricoreGameOver();
            break;
    }

    display.display();
}



void handleTetricoreInput() {
    if (millis() - lastInput < INPUT_DELAY) return;

    softDrop = isHeld(BTN_DOWN);

    if (isHeld(BTN_LEFT)) {
        moveLeft();
        lastInput = millis();
    }
    if(isHeld(BTN_RIGHT)) {
        moveRight();
        lastInput = millis();
    }

    static bool rotatePressedLast = true;
    bool rotateNow = isPressed(BTN_B);

    if (rotateNow && !rotatePressedLast) {
        tryRotate();
    }
    rotatePressedLast = rotateNow;
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
    for (int y = GRID_H - 1; y >= 0; y--) {
        if (isRowFull(y)) {
            clearRow = y;
            clearState = CLEAR_FLASH;
            clearStart = millis();
            return;
        }
    }
}

void actuallyClearRow(int y) {
    score += 100;
    
    for (int ty = y; ty > 0; ty--) {
        for (int x = FIELD_X; x < FIELD_X + FIELD_W; x++) {
        grid[ty][x] = grid[ty-1][x];
        }
    }
    for (int x = FIELD_X; x < FIELD_X + FIELD_W; x++) {
        grid[0][x] = 0;
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

    next.type = (Shapes)random(0,7);
    next.rotation = 0;

    if (!canMove(active.x, active.y)) {
        tetricoreState = TETRICORE_OVER;
        if (score > highScore) {
            highScore = score;
            EEPROM.put(TETRICORE_HIGHSCORE_ADDR, highScore);
            EEPROM.commit();
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

    display.drawRect(px, py, CELL_SIZE, CELL_SIZE, SH110X_WHITE);
    display.fillRect(px + 2, py + 2, CELL_SIZE - 4, CELL_SIZE - 4, SH110X_WHITE);
}

void drawTetricoreGame() {
    bool flashOn = ((millis() / 100) % 2) == 0;

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

    display.drawLine(left, 0, left, SCREEN_HEIGHT, SH110X_WHITE);
    display.drawLine(right, 0, right, SCREEN_HEIGHT, SH110X_WHITE);

    int uiX = (FIELD_X + FIELD_W) * CELL_SIZE + 8; // right of field
    int uiY = 4;

    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);

    display.setCursor(uiX, uiY);
    display.print("SCORE ");
    // display.setCursor(uiX, uiY + 10);
    display.print(score);

    display.setCursor(uiX, uiY + 10);
    display.print("HI ");
    // display.setCursor(uiX, uiY + 32);
    display.print(highScore);

    for (int sy = 0; sy < SHAPE_SIZE; sy++) {
        for (int sx = 0; sx < SHAPE_SIZE; sx++) {
            if (getCell(next.type, next.rotation, sx, sy)) {
                int px = PREVIEW_X + sx * CELL_SIZE;
                int py = PREVIEW_Y + sy * CELL_SIZE;

                display.drawRect(px, py, CELL_SIZE, CELL_SIZE, SH110X_WHITE);
                display.fillRect(px + 2, py + 2, CELL_SIZE - 4, CELL_SIZE - 4, SH110X_WHITE);
            }
        }
    }
}

void drawTetricoreTitle() {
    display.setTextSize(2);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(10, 10);
    display.println("Tetricore");
    
    display.setTextSize(1);
    display.setCursor(20, 40);
    display.print("High: ");
    display.print(highScore);

    display.setCursor(20, 50);
    display.println("Press A to start");
}

void drawTetricoreGameOver() {
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

void drawTetricorePause() {
    display.setTextSize(1);
    display.setCursor(32, 32);
    display.print("PAUSED");
}
