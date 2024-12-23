// Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License
#include <stdint.h>
#include <stdbool.h>
#include "util.h"
#include "gpio.h"

#define LED_RED      23
#define LED_YELLOW   24
#define LED_GREEN    25

NOINLINE void busywait(unsigned int n) {
    volatile unsigned count = 0;
    for(unsigned i=0; i<n; i++) {
        count++;
    }
}

const unsigned wait_0 = 0x200000;
const unsigned wait_1 = 0x800000;
const unsigned wait_2 = 0xF00000;

typedef void (*ProcessFn)(void);
void dispatch(ProcessFn,unsigned * stack);

void green_process(void);

void c_reset_handler(void) {
    gpio_set_func(LED_RED,GPF_OUTPUT);
    gpio_set_func(LED_YELLOW,GPF_OUTPUT);
    gpio_set_func(LED_GREEN,GPF_OUTPUT);

    // A little LED animation on reset
    for(int i=0; i<12; i++) {
        gpio_write(LED_RED,i%3==0);
        gpio_write(LED_YELLOW,i%3==1);
        gpio_write(LED_GREEN,i%3==2);
        busywait(0x100000);
    }
    gpio_write(LED_RED,0);
    gpio_write(LED_YELLOW,0);
    gpio_write(LED_GREEN,0);

    // more initialization stuff here

    // run green_process in user mode
    dispatch(green_process,(unsigned *)0x10000);
    // should never get here
}

#define SWI_BUSYWAIT 0

void c_swi_handler(int swi_num, const unsigned * registers) {
    // turn ON red LED indicating that we're in Supervisor mode
    gpio_write(LED_RED,1);

    switch(swi_num) {
    case SWI_BUSYWAIT:
        busywait(registers[0]);
        break;
    default:
        // invalid
        break;
    }

    // turn OFF red LED indicating that we're returning to User mode
    gpio_write(LED_RED,0);
}

// System call to busy wait
void swi_busywait(unsigned busy);

// Application code

void green_process(void) {
    int pin = LED_GREEN;
    while(true) {
        // ON
        gpio_write(pin,1);
        swi_busywait(wait_1);
        busywait(wait_1);
        // OFF
        gpio_write(pin,0);
        swi_busywait(wait_0);
        busywait(wait_0);
    }
}

