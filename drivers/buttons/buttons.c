#include <pico/stdlib.h>

#include "buttons.h"
#include "platform/platform_config.h"

static const int pins[] = {BTN_UP_PIN, BTN_DOWN_PIN, BTN_LEFT_PIN, BTN_RIGHT_PIN, BTN_A_PIN, BTN_B_PIN};

void buttons_init(void) {
    for (int i = 0; i < 6; i++) {
        gpio_init(pins[i]);
        gpio_set_dir(pins[i], GPIO_IN);
        gpio_pull_up(pins[i]); //active low
    }
}

bool buttons_read(int button_id) {
    return !gpio_get(pins[button_id]); //invert because pull up
}