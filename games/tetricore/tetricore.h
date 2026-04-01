#ifndef TETRICORE_H
#define TETRICORE_H

#include <stdint.h>

typedef enum {TETRICORE_TITLE, TETRICORE_PLAYING, TETRICORE_PAUSE, TETRICORE_OVER, TETRICORE_WARN_EXIT} TetricoreStates;

typedef enum {I, O, T, S, Z, J, L} Shapes;

typedef struct {
    int x;
    int y;
    Shapes type;
    uint8_t rotation;
} Piece;

typedef enum { CLEAR_NONE, CLEAR_FLASH } ClearAnimState;

void initTetricore(void);
void updateTetricore(void);
void drawTetricore(void);

#endif