// Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License
#include <stdint.h>
#include <stdbool.h>
#include "gpio.h"

// For RPi GPIO details, see https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf

// There are 54 GPIO "lines", 0 - 53

// Base memory address of GPIO controller
volatile uint32_t * const GPIO_BASE = (uint32_t *)0x20200000;

// GPIO Registers - GPIO has 41 32-bit registers

// Function select registers
#define GPFSEL0 0  // function select 0 - GPIO 0-9
#define GPFSEL1 1  // function select 1 - GPIO 10-19
#define GPFSEL2 2  // function select 2 - GPIO 20-29
#define GPFSEL3 3  // function select 3 - GPIO 30-39
#define GPFSEL4 4  // function select 4 - GPIO 40-49
#define GPFSEL5 5  // function select 5 - GPIO 50-53

// Output set/clear registers
#define GPSET0  7  // set bank 0
#define GPSET1  8  // set bank 1
#define GPCLR0  10 // clear bank 0
#define GPCLR1  11 // clear bank 2

int gpio_set_func(unsigned int gpio, unsigned func) {
    if(gpio>=GPIO_MAX) {
        return 1;
    }
    if(func>=GPF_MAX) {
        return 2;
    }
    int reg = gpio / 10;
    int shift = (gpio%10) * 3;
    GPIO_BASE[GPFSEL0+reg] = (GPIO_BASE[GPFSEL0+reg] & ~(7<<shift)) | (func<<shift);
    return 0;
}

int gpio_write(unsigned int gpio, unsigned int level) {
    if(gpio>=GPIO_MAX) {
        return 1;
    }
    uint32_t bank = (gpio>>5) ? 1 : 0; // determine which bank (0 or 1)
    uint32_t bit = (1<<(gpio&0x1F));   // determine bit within bank 
    if(level) {
        // Set GPIO
        GPIO_BASE[GPSET0+bank] = bit;
    } else {
        // Clear GPIO
        GPIO_BASE[GPCLR0+bank] = bit;
    }
    return 0;
}
