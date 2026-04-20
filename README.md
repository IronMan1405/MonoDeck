# MonoDeck v2

A compact, handheld game console built around the Raspberry Pi Pico 2 W, featuring a 128×64 OLED display and a custom game engine with a registry-based architecture.

---

## Games

| Game | Description |
|------|-------------|
| Snake | Classic snake |
| Pong | One-paddle pong |
| Breakout | Block-breaking arcade game |
| Tetricore | Tetris-style falling block game |
| Flappy | Flappy bird style arcade runner game |

---

## Hardware

### Bill of Materials

| Component | Details |
|-----------|---------|
| Microcontroller | Raspberry Pi Pico 2 W |
| Display | SH110x 128×64 OLED (I2C) |
| PCB | Custom PCB |
| Prototyping | Breadboard |
| Enclosure | 3D printed case |
| Power | Li-Po battery |

### Wiring

The display communicates over I2C. Default pin configuration is defined in `platform/platform_config.h`.
| Component Pin | Pico GPIO | 
|---------------|-----------|
| OLED SDA | GPIO 8 |
| OLED SCL | GPIO 9 |
| Up Button | GPIO 11 |
| Down Button | GPIO 12 |
| Left Button | GPIO 13 |
| Right Button | GPIO 10 |
| A Button | GPIO 20 |
| B Button | GPIO 21 |
| SEL Button | GPIO 3 |
| START Button | GPIO 2 |

---

## Project Structure

```
MonoDeck_v2/
├── MonoDeck_v2.c          # Entry point / main loop
├── app_registry/
│   ├── app_registry.h     # Game struct definition, extern declarations
│   └── app_registry.c     # Game registry table — only file to edit when adding a game
├── core/
│   ├── app_state.h/.c     # AppState enum (BOOT, MENU, IN_GAME)
│   ├── engine.h/.c        # launchGame, tickGame, requestExitToMenu
│   ├── input.h/.c         # Button input handling
│   └── boot.h/.c          # Boot sequence
├── menu/
│   └── menu.h/.c          # Scrollable game menu
├── games/
│   ├── snake/
│   ├── pong/
│   ├── breakout/
│   └── tetricore/
├── drivers/
│   ├── sh110x/            # OLED display driver
│   └── buttons/           # button interface
├── platform/
│   ├── platform_config.h  # Pin definitions, I2C config
│   ├── platform_init.h/.c # Hardware initialisation
│   └── platform_time.h/.c # Timing utilities
├── hardware/
│   ├── pcb/
│   │   └── monodeck/
│   │       ├── *.kicad_pro
│   │       ├── *.kicad_sch
│   │       ├── *.kicad_pcb
│   │       └── (other KiCad files)
│   └── cad/
└── assets/                # Bitmaps, fonts
```

---

## Build Instructions

### Prerequisites

- [Raspberry Pi Pico SDK v2.2.0](https://github.com/raspberrypi/pico-sdk)
- CMake ≥ 3.13
- ARM GCC toolchain `14_2_Rel1` (`arm-none-eabi-gcc`)
- [picotool v2.2.0](https://github.com/raspberrypi/picotool)

The easiest way to get all of these is via the [Raspberry Pi Pico VS Code Extension](https://marketplace.visualstudio.com/items?itemName=raspberry-pi.raspberry-pi-pico), which installs the SDK, toolchain, and picotool automatically under `~/.pico-sdk/`.

### Build

```bash
git clone https://github.com/IronMan1405/MonoDeck_v2
cd MonoDeck_v2
mkdir build && cd build
cmake ..
make
```

This produces `MonoDeck_v2.uf2` in the `build/` directory.

> **Note:** The CMakeLists is configured for `pico2_w`. If you are using a standard Pico W, change `set(PICO_BOARD pico2_w ...)` to `set(PICO_BOARD pico_w ...)` before building.

### Flash

**Option A — VS Code (recommended)**
 
If you're using the Raspberry Pi Pico VS Code Extension:
 
1. Open the project in VS Code
2. Click the **Run** button in the Pico sidebar (or use `Ctrl+Shift+P` → `Raspberry Pi Pico: Run Project`)
3. The extension builds, flashes, and reboots the device automatically via picotool — no manual BOOTSEL needed
 
**Option B — Manual UF2**

1. Hold the **BOOTSEL** button on the Pico W and connect via USB
2. It mounts as a mass storage device
3. Copy `MonoDeck_v2.uf2` onto it — it will reboot automatically

---

## Adding a New Game

The registry architecture means adding a game requires changes to **one file only**.

### 1. Create your game files

```
games/
└── mygame/
    ├── mygame.h
    └── mygame.c
```

Your game must expose three functions:

```c
void initMyGame(void);
void updateMyGame(void);
void drawMyGame(void);
```

To exit back to the menu from within a game, call:

```c
#include "core/engine.h"

requestExitToMenu();
```

### 2. Register the game

Open `app_registry/app_registry.c` and add one `#include` and one entry:

```c
#include "games/mygame/mygame.h"   // add this

Game games[] = {
    {"Snake",     initSnake,     updateSnake,     drawSnake},
    {"Pong",      initPong,      updatePong,      drawPong},
    {"Breakout",  initBreakout,  updateBreakout,  drawBreakout},
    {"Tetricore", initTetricore, updateTetricore, drawTetricore},
    {"MyGame",    initMyGame,    updateMyGame,    drawMyGame},  // add this
};
```

### 3. Add highscore storage for the game

Open `platform/storage_map.h` and add the entry:

```c
typedef struct {
    uint32_t magic;
    uint16_t snake_hs;
    uint16_t pong_hs;
    uint16_t breakout_hs;
    uint16_t tetricore_hs;
    uint16_t mygame_hs;    // add this
    uint8_t  _pad[256 - 12];
} StoragePage;

#define STORAGE_MYGAME_HIGHSCORE_OFFSET 0x008   // add this, make sure to keep the offset of the new game by 2 from the previous game
```

Next open `platform/platform_storage.c` in the definition of ```loadStorage()``` add:

```c
void loadStorage(void) {
    ...

    if (page.magic == STORAGE_MAGIC) {
        gStorage = page;
    } else {
        gStorage.magic = STORAGE_MAGIC;
        gStorage.snake_hs = 0;
        gStorage.pong_hs = 0;
        gStorage.breakout_hs = 0;
        gStorage.tetricore_hs = 0;
        gStorage.mygame_hs = 0;     // add this
    }
}
```

And initialize it in your game's init function in `games/mygame/mygame.c`:

```c
#include "platform/platform_storage.h"

void initMyGame(void) {
    ...

    loadStorage();
    highScore = gStorage.mygame_hs;

    if (highScore < 0 || highScore > 999) highScore = 0;

    ...
}
```

*(Note: you can set the upper limit of highscore to anything, i have set it as 999 for most games, and 99999 for tetricore)*

---

## ToDo

- [x] Add Select & Start buttons interface
- [x] Add persistent high score storing
- [x] Refine Scoring methods
- [ ] Add levels & game modes
- [x] Add PCB files 
- [x] Add enclosure CAD files 
- [x] Implement Bi-directional rotation & wall kicks in Tetricore
- [x] UI/UX enhancements
- [x] Fix reverse scroll

---

## License

MIT
