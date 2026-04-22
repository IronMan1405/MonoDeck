#ifndef CONWAYSGOL_H
#define CONWAYSGOL_H

typedef enum {CONWAY_PLAYING, CONWAY_PAUSE, CONWAY_TITLE, CONWAY_OVER} ConwayState;

void initConway(void);
void updateConway(void);
void drawConway(void);

#endif