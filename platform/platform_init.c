#include "platform_init.h"
#include "platform_config.h"

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"

void platform_init(void) {
    stdio_init_all();
    i2c_init(I2C_PORT, I2C_BAUDRATE);

    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);

    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    const uint8_t buttons[] = {
        BTN_UP_PIN,
        BTN_DOWN_PIN,
        BTN_LEFT_PIN,
        BTN_RIGHT_PIN,
        BTN_A_PIN,
        BTN_B_PIN
    };

    for (int i = 0; i < sizeof(buttons); i++) {
        gpio_init(buttons[i]);
        gpio_set_dir(buttons[i], GPIO_IN);
        gpio_pull_up(buttons[i]);
    }
}