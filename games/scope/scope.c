#include <stdbool.h>
#include <stdint.h>

#include "scope.h"
#include "drivers/sh110x/sh110x.h"
#include "core/engine.h"
#include "core/input.h"

#define SAMPLE_COUNT 512

#define GRAPH_X 2
#define GRAPH_Y 10
#define GRAPH_WIDTH 124
#define GRAPH_HEIGHT 36

#define SAMPLE_MAX 4095

static uint16_t samples[SAMPLE_COUNT];

static bool frozen;
static bool canExit;
static uint8_t phase = 0;

static WaveformType waveform = WAVE_SINE;

static const uint8_t sineTable[32] = {
    128, 153, 177, 199, 218, 234, 246, 253,
    255, 253, 246, 234, 218, 199, 177, 153,
    128, 103,  79,  57,  38,  22,  10,   3,
      0,   3,  10,  22,  38,  57,  79, 103
};

static void generateSamples(void);

static void drawGrid(void);
static void drawWaveform(void);
static void drawHeader(void);
static void drawFooter(void);

void initScope(void) {
    frozen = false;
    canExit = false;
    phase = 0;

    waveform = WAVE_SINE;

    generateSamples();

    sh110x_clear();
    sh110x_update();
}

void updateScope(void) {
    if (!isHeld(BTN_B)) {
        canExit = true;
    }

    if (canExit && isPressed(BTN_B)) {
        canExit = false;
        requestExitToMenu();
        return;
    }

    if (isPressed(BTN_A)) {
        frozen = !frozen;
    }

    if (!frozen) {
        phase++;
        generateSamples();
    }

    if (isPressed(BTN_SEL)) {
        waveform = (waveform + 1) % WAVE_COUNT;
        phase = 0;
        generateSamples();
    }
}

void drawScope(void) {
    sh110x_clear();

    drawHeader();
    // drawGrid();
    drawWaveform();
    drawFooter();

    sh110x_update();
}





static void generateSamples(void) {
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        int position = (i + phase) % 32;
        int value = 128;

        switch (waveform) {
            case WAVE_SINE:
                value = sineTable[position];
                break;

            case WAVE_SQUARE:
                value = (position < 16) ? 255 : 0;
                break;

            case WAVE_TRIANGLE:
                value = (position < 16)
                    ? position * 16
                    : 255 - (position - 16) * 16;
                break;

            default:
                break;
        }

        samples[i] = 2048 + ((int)value - 128) * 11;
    }
}

static void drawGrid(void) {
    sh110x_draw_rect(GRAPH_X, GRAPH_Y, GRAPH_WIDTH, GRAPH_HEIGHT);

    // Vertical divisions.
    for (int x = GRAPH_X + 24; x < GRAPH_X + GRAPH_WIDTH; x += 24) {
        sh110x_draw_line(x, GRAPH_Y + 1, x, GRAPH_Y + GRAPH_HEIGHT - 2);
    }

    // Horizontal divisions.
    for (int y = GRAPH_Y + 9; y < GRAPH_Y + GRAPH_HEIGHT; y += 9) {
        sh110x_draw_line(GRAPH_X + 1, y, GRAPH_X + GRAPH_WIDTH - 2, y);
    }

    // Centre reference line.
    int centreY = GRAPH_Y + GRAPH_HEIGHT / 2;

    for (int x = GRAPH_X + 2; x < GRAPH_X + GRAPH_WIDTH - 2; x += 4) {
        sh110x_draw_pixel(x, centreY, true);
    }
}

static void drawWaveform(void) {
    int previousX = GRAPH_X + 1;
    int previousY = GRAPH_Y + GRAPH_HEIGHT / 2;

    for (int x = 0; x < GRAPH_WIDTH - 2; x++) {
        int sampleIndex = x * (SAMPLE_COUNT - 1) / (GRAPH_WIDTH - 3);

        int y = GRAPH_Y + GRAPH_HEIGHT - 2 - (samples[sampleIndex] * (GRAPH_HEIGHT - 3) / SAMPLE_MAX);

        int screenX = GRAPH_X + 1 + x;

        if (x > 0) {
            sh110x_draw_line(previousX, previousY, screenX, y);
        }

        previousX = screenX;
        previousY = y;
    }
}

static void drawHeader(void) {
    const char *waveNames[] = {
        "SIN",
        "SQR",
        "TRI"
    };

    sh110x_draw_text(2, 1, "SCOPE", 1);
    sh110x_draw_text(42, 1, waveNames[waveform], 1);
    sh110x_draw_text(78, 1, frozen ? "CH1 HOLD" : "CH1 LIVE", 1);
    sh110x_draw_line(0, 8, 127, 8);
}

static void drawFooter(void) {
    sh110x_draw_line(0, 47, 127, 47);

    sh110x_draw_text(2, 48, "1V/D 500uS/D", 1);
    sh110x_draw_text(2, 56, "A:HOLD SEL:WAVE", 1);
    sh110x_draw_text(98, 56, "B:EXIT", 1);
}