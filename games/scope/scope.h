#ifndef SCOPE_H
#define SCOPE_H

typedef enum {
    WAVE_SINE,
    WAVE_SQUARE,
    WAVE_TRIANGLE,
    WAVE_COUNT
} WaveformType;

void initScope(void);
void updateScope(void);
void drawScope(void);

#endif