#pragma once

#include <stdint.h>

#define STORAGE_FLASH_SIZE          (4 * 1024 * 1024)   // 4 MB
#define FLASH_SECTOR_SIZE           4096
#define STORAGE_FLASH_OFFSET        (STORAGE_FLASH_SIZE - FLASH_SECTOR_SIZE)

#define STORAGE_MAGIC 0xDEADBEEF

typedef struct {
    uint32_t magic;
    uint16_t snake_hs;
    uint16_t pong_hs;
    uint16_t breakout_hs;
    uint16_t tetricore_hs;
    uint8_t  _pad[256 - 12];
} StoragePage;

#define STORAGE_HIGHSCORE_SIZE izeof(uint16_t) //2bytes

#define STORAGE_SNAKE_HIGHSCORE_OFFSET 0x000
#define STORAGE_PONG_HIGHSCORE_OFFSET 0x002
#define STORAGE_BREAKOUT_HIGHSCORE_OFFSET 0x004
#define STORAGE_TETRICORE_HIGHSCORE_OFFSET 0x006

#define STORAGE_BASE_ADDR ((const uint8_t *)(XIP_BASE + STORAGE_FLASH_OFFSET))
#define STORAGE_ADDR(offset) ((const uint8_t *)(XIP_BASE + STORAGE_FLASH_OFFSET + (offset)))