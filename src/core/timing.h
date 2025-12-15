#pragma once
#include <Arduino.h>

extern unsigned long lastFrameTime;
const unsigned long FRAME_TIME = 50;

bool frameReady();