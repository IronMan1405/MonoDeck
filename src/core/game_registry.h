#pragma once

struct Game {
    void (*init)();
    void (*update)();
    void (*draw)();
};