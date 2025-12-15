#include "display.h"
#include <Wire.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void initDisplay() {
  display.begin(0x3C, true);
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.display();
}