// Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License
#include "toast.h"
#include "util.h"
#include "gpio.h"

void busywait(unsigned int n) {
    volatile unsigned count = 0;
    for(unsigned i=0; i<n; i++) {
        count++;
    }
}

void panic(int code) {
    while(1) {
        for(int i=0;i<code;i++) {
            gpio_write(SYS_LED_RED,1);
            busywait(0x200000);
            gpio_write(SYS_LED_RED,0);
            busywait(0x200000);
        }
        busywait(0x800000);
    }
}