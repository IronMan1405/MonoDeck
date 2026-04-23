#ifndef MINES_H
#define MINES_H

#include <stdbool.h>

typedef enum {CLOSED, OPENED, FLAGGED} CellState;

typedef struct {
    bool isMine;
    int adjacent;
    CellState state;
} CELL;

typedef struct {
    int col;
    int row;
} SELECTED;


typedef enum {MINES_PLAYING, MINES_OVER, MINES_TITLE, MINES_PAUSE, MINES_EXIT_WARN, MINES_REVEAL} MinesState;


void initMines(void);
void updateMines(void);
void drawMines(void);

#endif