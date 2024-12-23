// Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License
/*
 * TOAST
 *
 * Public interface to the TOAST Operating System
 * 
 */
#ifndef __TOAST_H__
#define __TOAST_H__

#include <stdbool.h>

#define SYS_LED_BLUE     22
#define SYS_LED_RED      23
#define SYS_LED_YELLOW   24
#define SYS_LED_GREEN    25

// System calls
void sys_set_led(unsigned int led, bool on);
void sys_yield();
#endif // __SYS_H__
