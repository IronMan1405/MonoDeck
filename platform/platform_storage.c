#include <string.h>

#include "platform_storage.h"
#include "storage_map.h"

#include "pico/flash.h"
#include "hardware/flash.h"
#include "hardware/sync.h"

StoragePage gStorage = {0};

typedef struct {
    uint32_t offset;
    StoragePage page;
} FlashParams;

static void do_flash_write(void *param) {
    FlashParams *p = (FlashParams *)param;

    flash_range_erase(p->offset, FLASH_SECTOR_SIZE);
    flash_range_program(p->offset, (uint8_t *)&p->page, 256);
}

void loadStorage(void) {
    StoragePage page;

    memcpy(&page, STORAGE_BASE_ADDR, sizeof(page));

    if (page.magic == STORAGE_MAGIC) {
        gStorage = page;
    } else {
        gStorage.magic = STORAGE_MAGIC;
        gStorage.snake_hs = 0;
        gStorage.pong_hs = 0;
        gStorage.breakout_hs = 0;
        gStorage.tetricore_hs = 0;
        gStorage.flappy_hs = 0;
        gStorage.mines_hs = 0;
    }
}

void updateStorage(void) {
    gStorage.magic = STORAGE_MAGIC;
    FlashParams params = {
        .offset = STORAGE_FLASH_OFFSET,
        .page = gStorage,
    };
    flash_safe_execute(do_flash_write, &params, UINT32_MAX);
}
