#include "engine.h"
#include "core/app_state.h"
#include "app_registry/app_registry.h"
#include <string.h>

bool gameWantsExit = false;

static const Game* currentGame = NULL;

void launchGame(const char* name) {
    currentGame = registry_find(name);

    if (currentGame) {
        currentGame->init();
        currentState = IN_GAME;
    }
}

void tickGame(void) {
    if (currentGame) {
        currentGame->update();
        currentGame->draw();
    }
}

void requestExitToMenu() {
    gameWantsExit = true;
    currentGame = NULL;
}