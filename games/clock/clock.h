#ifndef CLOCK_H
#define CLOCK_H

typedef enum {CLOCK_RUNNING, CLOCK_EXIT_WARN} ClockState;

void initClock(void);
void updateClock(void);
void drawClock(void);

#endif