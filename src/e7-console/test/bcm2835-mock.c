#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "bcm2835.h"
#include "str.h"

int gpio_set_func(unsigned int gpio, unsigned func) {
    return 0;
}

int gpio_write(unsigned int gpio, unsigned int level) {
    return 0;
}

void uart_init(Baud baud) {
    
}

void uart_putc(uint8_t c) {
    putc(c,stdout);
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
    return getc(stdin);
}

uint64_t system_timer(void) {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}

void panic(int code) {
    printf("PANIC: code=%d\n",code);
    exit(code);
}

uint32_t _proc_main(uint32_t entrypoint, uint32_t init_param) {
    // mock
    return 0;
}

