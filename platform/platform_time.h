#ifndef PLATFORM_TIME_H
#define PLATFORM_TIME_H

#include <stdint.h>

uint32_t platform_millis(void);
bool platform_frame_ready(void);

#endif