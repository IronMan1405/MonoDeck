#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "platform_config.h"
#include "platform_init.h"
#include "sh110x.h"

int main() {
    platform_init();

    sleep_ms(5000);

    printf("Monodeck v2 booting...\n");

    uint8_t dummy = 0x00;
    int ret = i2c_write_blocking(I2C_PORT, DISPLAY_I2C_ADDRESS, &dummy, 1, false);

    if (ret < 0) {
        printf("OLED NOT responding on I2C\n");
    } else {
        printf("OLED ACKed on I2C\n");
    }
    
    sh110x_init();
    printf("OLED init done\n");
    
    sh110x_clear();
    sh110x_draw_text(20, 20, "Monodeck v2", 1);
    sh110x_update();

    while (1) {
        tight_loop_contents();
    }
}
