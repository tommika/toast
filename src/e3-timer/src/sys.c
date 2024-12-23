// Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License
#include <stdint.h>
#include <stdbool.h>
#include "util.h"
#include "sys.h"
#include "gpio.h"
#include "interrupts.h"

typedef void (*ProcessFn)(void);
void dispatch(ProcessFn,unsigned * stack);

void app_main(void);

uint32_t timer_cb(uint32_t);

// A little LED animation on reset
void animate_leds(int iters) {
    for(int i=0; i<iters; i++) {
        gpio_write(SYS_LED_RED,i%3==0);
        gpio_write(SYS_LED_YELLOW,i%3==1);
        gpio_write(SYS_LED_GREEN,i%3==2);
        busywait(0x80000);
    }
    // Clear LEDs
    gpio_write(SYS_LED_RED,0);
    gpio_write(SYS_LED_YELLOW,0);
    gpio_write(SYS_LED_GREEN,0);
}

void c_reset_handler(void) {
    gpio_set_func(SYS_LED_RED,GPF_OUTPUT);
    gpio_set_func(SYS_LED_YELLOW,GPF_OUTPUT);
    gpio_set_func(SYS_LED_GREEN,GPF_OUTPUT);
    
    animate_leds(16);

    // Start timer; this will cause the yellow LED to blink when
    // interrupts are enabled
    timer_set(0x400,timer_cb);

    // Run the "application" in User mode (w/ interrupts enabled)
    dispatch(app_main,(unsigned *)0x10000);
}

uint32_t timer_cb(uint32_t val) {
    static bool on = true;
    gpio_write(SYS_LED_YELLOW,on);
    on = !on;
    return 0;
}

#define SWI_BUSYWAIT 0

void c_swi_handler(int swi_num, const unsigned * registers) {
    // Turn ON red LED indicating that we're in Supervisor mode
    // We should observe that the timer interrupts are suspended
    // while we're servicing the swi request (yellow light will stop flashing)
    gpio_write(SYS_LED_RED,1);

    switch(swi_num) {
    case SWI_BUSYWAIT:
        busywait(registers[0]);
        break;
    default:
        // invalid
        break;
    }

    // Turn OFF red LED indicating that we're returning to User mode
    gpio_write(SYS_LED_RED,0);
}

