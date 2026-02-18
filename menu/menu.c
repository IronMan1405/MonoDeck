#include "menu.h"
#include "core/input.h"
#include "core/app_state.h"
#include "core/engine.h"
#include "drivers/sh110x/sh110x.h"
#include "platform/platform_config.h"

static int selected = 0;
static const int totalGames = 4;

void initMenu(void) {
    selected = 0;
}

void updateMenu(void) {
    if (isPressed(BTN_UP_PIN))
    {
        selected--;
        if (selected < 0)
            selected = totalGames - 1;
    }

    if (isPressed(BTN_DOWN_PIN))
    {
        selected++;
        if (selected >= totalGames)
            selected = 0;
    }

    if (isPressed(BTN_A))
    {
        launchGame(selected);
    }
}

void drawMenu(void) {
    sh110x_clear();

    const char* games[] = {
        "Snake",
        "Breakout",
        "Pong",
        "Tetricore"
    };

    for (int i = 0; i < totalGames; i++)
    {
        int y = 10 + (i * 12);

        if (i == selected)
            sh110x_draw_text(0, y, ">", 1);

        sh110x_draw_text(12, y, games[i], 1);
    }

    sh110x_update();
}
