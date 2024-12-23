// Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License
#include "assert.h"
#include "str.h"
#include "bcm2835.h"

void assert_failed(const char * expr, const char * filename, int lineno, int panic_code) {
    char buff[8];
    uart_puts("\033[31;1m");
    uart_puts("assertion failed @ ");
    uart_puts(filename);
    uart_puts(":");
    uart_puts(itoa(lineno,buff,10));
    uart_puts(": ");
    uart_puts(expr);
    uart_puts("\033[0m");
    uart_puts("\r\n");
    panic(panic_code);
}
