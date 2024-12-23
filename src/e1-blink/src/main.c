// Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License 
#include <stdint.h>
#include <stdbool.h>
#include "util.h"
#include "gpio.h"

#define LED_BLUE 22

NOINLINE void busy_wait(unsigned int n) {
    volatile unsigned count = 0;
    for(unsigned i=0; i<n; i++) {
        count++;
    }
}

void main(void) {
    gpio_set_func(LED_BLUE,GPF_OUTPUT);
    const int on_wait = 0x200000;
    const int off_wait = 0x800000;
    while(true) {
        gpio_write(LED_BLUE,0);
        busy_wait(on_wait);
        gpio_write(LED_BLUE,1);
        busy_wait(off_wait);
    }
}
