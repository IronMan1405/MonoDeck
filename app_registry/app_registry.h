#ifndef APP_REGISTRY_H
#define APP_REGISTRY_H

typedef struct {
    const char* name;
    void (*init)(void);
    void (*update)(void);
    void (*draw)(void);
} Game;

extern Game games[];
extern const int gameCount;

const Game* registry_find(const char* name);

#endif