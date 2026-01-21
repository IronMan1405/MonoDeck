#include <stdio.h>
#include "pico/stdlib.h"

int main() {
    stdio_init_all();

    // Give USB time to enumerate
    sleep_ms(2000);

    printf("MonoDeck Pico bring-up OK\n");

    while (true) {
        printf("MonoDeck Pico alive\n");
        sleep_ms(1000);
    }
}
