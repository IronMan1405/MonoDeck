#ifndef ENGINE_H
#define ENGINE_H

#include <stdbool.h>

extern bool gameWantsExit;

void launchGame(const char* name);
void tickGame(void);
void requestExitToMenu(void);

#endif
