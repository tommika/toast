// Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License
#include <stdint.h>
#include <stdbool.h>
#include "bcm2835.h"
#include "str.h"

void busywait(uint32_t n);

volatile IRQ_Registers * irq_registers = (IRQ_Registers*)(BASE_BUS_ADDR + IRQ_REGISTERS_OFFSET);
volatile System_Timer_Registers * system_timer_registers = (System_Timer_Registers*)(BASE_BUS_ADDR + SYSTEM_TIMER_OFFSET);
volatile Timer_Registers * timer_registers = (Timer_Registers*)(BASE_BUS_ADDR + TIMER_REGISTERS_OFFSET);
volatile GPIO_Registers * gpio_registers = (GPIO_Registers*)(BASE_BUS_ADDR + GPIO_REGISTERS_OFFSET);

int gpio_set_func(unsigned int gpio, unsigned func) {
    if(gpio>=GPIO_MAX) {
        return 1;
    }
    if(func>=GPF_MAX) {
        return 2;
    }
    int fsel = gpio / 10;
    int shift = (gpio%10) * 3;
    volatile uint32_t * reg = (&gpio_registers->fsel[fsel]);
    *reg =  (*reg & ~(7<<shift)) | (func<<shift);
    return 0;
}

int gpio_write(unsigned int gpio, unsigned int level) {
    if(gpio>=GPIO_MAX) {
        return 1;
    }
    uint32_t bank = (gpio>>5) ? 1 : 0; // determine which bank (0 or 1)
    uint32_t bit = (1<<(gpio&0x1F));   // determine bit within bank 
    volatile uint32_t * reg;
    if(level) {
        // Set GPIO
        reg = &gpio_registers->set[bank];
    } else {
        // Clear GPIO
        reg = &gpio_registers->clr[bank];
    }
    *reg = bit;
    return 0;
}

// BCM2835 section 2.2 - Mini UART
//    baudrate = (system_clock_freq) / (8 * ( baudrate_reg + 1 ) )
//    baudrate * 8 * ( baudrate_reg + 1 ) = system_clock_freq
//    baudrate_reg + 1 = system_clock_freq / (baudrate * 8)
//    baudrate_reg = system_clock_freq / (baudrate * 8) - 1
//    baudrate_reg = (system_clock_freq / (baudrate * 8)) - 1
volatile uint32_t * aux_registers = (uint32_t *)(BASE_BUS_ADDR + AUX_REGISTERS_OFFSET);
void uart_init(Baud baud) {
    gpio_set_func(14,GPF_ALT5);
    gpio_set_func(15,GPF_ALT5);
    // See BCM2835 - Table 6-28 synopsis
    gpio_registers->pud = GPPUD_OFF;
    busywait(150);
    gpio_registers->pudclk[0] = (1<<14)|(1<<15);
    busywait(150);
    gpio_registers->pudclk[0] = 0;

    aux_registers[AUX_ENABLES] = 1;         // enable the mini-uart 
    aux_registers[AUX_MU_CNTL_REG] = 0;     // disable tx/rx 
    aux_registers[AUX_MU_IER_REG] = 0;      // disable tx/rx interrupts (not now)
    aux_registers[AUX_MU_LCR_REG] = 3;      // 8-bit mode
    aux_registers[AUX_MU_MCR_REG] = 0;
    aux_registers[AUX_MU_BAUD_REG] = 270;   // Set baud rate to 115200
    aux_registers[AUX_MU_CNTL_REG] = 3;     // enable tx/rx 
}

void uart_putc(uint8_t c) {
    // wait until device is ready to accept data
    while(1) {
        volatile uint32_t val = aux_registers[AUX_MU_LSR_REG];
        if(val & 0x20) {
            break;
        }
    }
    aux_registers[AUX_MU_IO_REG] = c;
}

void uart_puts(const char * str) {
    while(*str) {
        uart_putc(*str++);
    }
}

void uart_putn(int val) {
    char buff[19];
    itoa(val,buff,10);
    uart_puts(buff);
}

uint8_t uart_getc() {
    while(1) {
        volatile uint32_t val = aux_registers[AUX_MU_LSR_REG];
        if(val & 0x01) {
            break;
        }
    }
    return aux_registers[AUX_MU_IO_REG] & 0xFF;
}

static inline uint64_t cur_time_micros(void) {
    uint64_t timer = system_timer_registers->chi;
    timer = (timer<<32) + system_timer_registers->clo;
    return timer;
}

// Return system timer microseconds
uint64_t system_timer(void) {
    return cur_time_micros();
}

uint32_t busy_wait_millis(uint32_t millis) {
    uint64_t deadline = system_timer() + (millis*1000);
    while(cur_time_micros()<deadline);
    return millis;
}

void panic(int code) {
    char buff[8];
    uart_puts("\033[31;1m");
    uart_puts("panic: code=");
    uart_puts(itoa(code,buff,10));
    uart_puts("\033[0m");
    uart_puts("\r\n");
    gpio_set_func(SYS_LED_BLUE,GPF_OUTPUT);
    while(1) {
        for(int i=0;i<code;i++) {
            gpio_write(SYS_LED_BLUE,1);
            busy_wait_millis(250);
            gpio_write(SYS_LED_BLUE,0);
            busy_wait_millis(250);
        }
        busy_wait_millis(1000);
    }
}
