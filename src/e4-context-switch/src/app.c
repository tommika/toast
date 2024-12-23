// Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License
#include <stdint.h>
#include "toast.h"
#include "util.h"
#include "gpio.h"

static void flash(uint32_t led, unsigned times) {
    while(times--) {
        // ON
        gpio_write(led,1);
        busywait(0x40000);
        // OFF
        gpio_write(led,0);
        busywait(0x40000);
    }
    busywait(0x800000);
}

void green_blinker() {
    volatile int x = 5150;
    volatile int y = x+1;
    volatile int z = y+1;
    flash(SYS_LED_GREEN,10);
    while(1) {
        // Check that we have the correct stack
        if(x!=5150 || z!=(y+1) ) {
            flash(SYS_LED_GREEN,-1);
        }
        y++;
        z++;

        // ON
        sys_set_led(SYS_LED_GREEN,1);
        busywait(0x800000);
        sys_yield();
        // OFF
        sys_set_led(SYS_LED_GREEN,0);
        busywait(0x200000);
        sys_yield();
    }
}

void yellow_blinker() {
    volatile int x = 2020;
    volatile int y = x+1;
    volatile int z = y+1;

    flash(SYS_LED_YELLOW,10);
    while(1) {
        // Check that we have the correct stack
        if(x!=2020 || z!=(y+1) ) {
            flash(SYS_LED_YELLOW,-1);
        }
        y++;
        z++;

        // ON
        sys_set_led(SYS_LED_YELLOW,1);
        busywait(0x200000);
        sys_yield();
        // OFF
        sys_set_led(SYS_LED_YELLOW,0);
        busywait(0x100000);
        sys_yield();
    }
}
