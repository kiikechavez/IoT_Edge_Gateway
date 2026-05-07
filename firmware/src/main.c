/*
 * IoT Edge Gateway - punto de entrada
 * Brando Enrique Chavez Vergara
 */
#include <stdio.h>
#include "pico/stdlib.h"

int main(void) {
    stdio_init_all();
    while (true) {
        printf("IoT Edge Gateway booting...\n");
        sleep_ms(1000);
    }
}
