#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "hardware/adc.h"

#include "conwaysgol.h"
#include "drivers/sh110x/sh110x.h"
#include "core/app_state.h"
#include "core/engine.h"
#include "core/input.h"
#include "platform/platform_time.h"
#include "platform/platform_storage.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define COLS 128
#define ROWS 64
#define CELL_SIZE 10
#define FPS 10

#define WIDTH COLS * CELL_SIZE
#define HEIGHT ROWS * CELL_SIZE

static bool canExit = true;

static unsigned long lastStepTime;
static unsigned long frameInterval = 100;

static uint32_t startTime = 0;
static uint32_t elapsed = 0;
static uint32_t population = 0;

ConwayState conwayState = CONWAY_TITLE;

static bool grid[ROWS][COLS];

static void clearGrid();
static void generateCells();
static void getNeighbors(int x, int y, int neighbors[][2], int *count);
static void updateGrid();
static void handleConwayInput();

static void drawConwayGame();
static void drawConwayTitle();
static void drawConwayPause();
static void drawConwayOver();

void initConway(void) {
    uint32_t seed = adc_read() ^ platform_millis();
    srand(seed);

    sh110x_clear();
    sh110x_update();

    canExit = false;
    
    conwayState = CONWAY_TITLE;

    lastStepTime = platform_millis();
}

void updateConway(void) {
    switch (conwayState) {
        case CONWAY_TITLE:
            if (isPressed(BTN_START)) {
                generateCells();
                startTime = platform_millis();
                conwayState = CONWAY_PLAYING;
            }
            if (!isHeld(BTN_B)) {
                canExit = true;
            }
            if (canExit && isPressed(BTN_B)) {
                canExit = false;
                requestExitToMenu();
            }
            break;
        case CONWAY_PLAYING:
            handleConwayInput();

            if (platform_millis() - lastStepTime >= frameInterval) {
                lastStepTime = platform_millis();
                updateGrid();
            }

            if (isPressed(BTN_START)) {
                conwayState = CONWAY_PAUSE;
            }
            break;
        case CONWAY_PAUSE:
            if (isPressed(BTN_START)) {
                conwayState = CONWAY_PLAYING;
            }
            if (canExit && isPressed(BTN_B)) {
                elapsed = (platform_millis() - startTime) / 1000;
                conwayState = CONWAY_OVER;
            }
            break;
        case CONWAY_OVER:
            if (!isHeld(BTN_B)) {
                canExit = true;
            }
            if (canExit && isPressed(BTN_B)) {
                canExit = false;
                requestExitToMenu();
            }

            if (isPressed(BTN_A)) {
                initConway();
                conwayState = CONWAY_TITLE;
                lastStepTime = platform_millis();
                break;
            }
            break;
    }
}

void drawConway(void) {
    sh110x_clear();
    
    switch (conwayState) {
        case CONWAY_TITLE:
            drawConwayTitle();
            break;
        case CONWAY_PLAYING:
            drawConwayGame();
            break;
        case CONWAY_PAUSE:
            drawConwayPause();
            break;
        case CONWAY_OVER:
            drawConwayOver();
            break;
    }
    sh110x_update();
}





static void handleConwayInput() {
    if (isPressed(BTN_A)) {
        generateCells();
        startTime = platform_millis();
    }
    if (isPressed(BTN_B)) {
        clearGrid();
    }
}

static void clearGrid() {
    memset(grid, 0, sizeof(grid));
}

static void generateCells() {
    memset(grid, 0, sizeof(grid));
    int n = (rand() % 6 + 4) * COLS;

    for (int i = 0; i < n; i++) {
        int x = rand() % COLS;
        int y = rand() % ROWS;
        grid[y][x] = true;
    }
}

static void getNeighbors(int x, int y, int neighbors[][2], int *count) {
    *count = 0;

    for (int dx = -1; dx <= 1; dx++) {
        if (x + dx >= COLS || x + dx < 0) continue;
        for (int dy = -1; dy <= 1; dy++) {
            if (y + dy >= ROWS || y + dy < 0) continue;
            if (dx == 0 && dy == 0) continue;

            neighbors[*count][0] = x + dx;
            neighbors[*count][1] = y + dy;
            (*count)++;
        }
    }
}

static void updateGrid() {
    static bool all_neighbors[ROWS][COLS];
    static bool new_positions[ROWS][COLS];
    memset(new_positions, 0, sizeof(new_positions));
    memset(all_neighbors, 0, sizeof(all_neighbors));

    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            if (!grid[y][x]) continue;

            int neighbors[8][2], count;
            
            getNeighbors(x, y, neighbors, &count);

            for (int i = 0; i < count; i++) {
                all_neighbors[neighbors[i][1]][neighbors[i][0]] = true;
            }

            int live = 0;
            for (int i = 0; i < count; i++) {
                if (grid[neighbors[i][1]][neighbors[i][0]]) live++;
            }

            if (live == 2 || live == 3) {
                new_positions[y][x] = true;
            }
        }
    }

    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            if (!all_neighbors[y][x]) continue;

            int neighbors[8][2], count;
            
            getNeighbors(x, y, neighbors, &count);
            
            int live = 0;
            for (int i = 0; i < count; i++) {
                if (grid[neighbors[i][1]][neighbors[i][0]]) live++;
            }
            if (live == 3) {
                new_positions[y][x] = true;
            }
        }
    }

    memcpy(grid, new_positions, sizeof(grid));

    population = 0;
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            if (grid[y][x]) population++;
        }
    }
}





static void drawConwayGame() {
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            if (grid[y][x]) sh110x_draw_pixel(x, y, true);
        }
    }
}

static void drawConwayTitle() {
    sh110x_draw_text(10, 10, "Conway", 2);
    sh110x_draw_text(20, 50, "Press START", 1);
}

static void drawConwayPause() {
    sh110x_draw_text(32, 12, "PAUSED", 2);
    sh110x_draw_text(2, 46, "START: Return to game", 1);
    sh110x_draw_text(2, 56, "B: Exit", 1);
}

static void drawConwayOver() {
    uint32_t mins = elapsed / 60;
    uint32_t secs = elapsed % 60;

    sh110x_draw_text(20, 10, "GAME ENDED", 2);
    
    char popbuf[20];
    snprintf(popbuf, sizeof(popbuf), "Population: %u", (unsigned)population);

    char timebuf[20];
    snprintf(timebuf, sizeof(timebuf), "Time: %02lu:%02lu", mins, secs);

    sh110x_draw_text(20, 32, popbuf, 1);
    sh110x_draw_text(20, 44, timebuf, 1);

    sh110x_draw_text(5, 56, "A:Title      B:Exit", 1);
}