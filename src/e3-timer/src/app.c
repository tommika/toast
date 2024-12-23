// Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License
#include "gpio.h"
#include "sys.h"
#include "util.h"

void app_main(void) {
    const unsigned wait_0 = 0x200000;
    const unsigned wait_1 = 0x800000;
    const unsigned wait_2 = 0xF00000;
    const int pin = SYS_LED_GREEN;
    unsigned long count = 0;
    while(1) {
        // ON
        gpio_write(pin,1);
        busywait(wait_1);
        // OFF
        gpio_write(pin,0);
        busywait(wait_0);
        count++;
        // Make a system call once in a while.
        if(count%5==0) {
            sys_busywait(wait_2);
        }
    }
}

