// Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License
/*
 * TOAST
 *
 * Public interface to the TOAST Operating System
 * 
 */
#ifndef __TOAST_H__
#define __TOAST_H__

#include <stdint.h>
#include <stdbool.h>

#define NULL ((void*)0)
#define SYS_LED_BLUE     22
#define SYS_LED_RED      23
#define SYS_LED_YELLOW   24
#define SYS_LED_GREEN    25

// System calls

uint32_t sys_set_led(uint32_t led, uint32_t val);
void sys_yield();
uint32_t sys_clock_millis();
uint32_t sys_sleep_millis(uint32_t millis);

typedef uint32_t (*ProcessMainFn)(uint32_t init_param);
int sys_fork(ProcessMainFn main, uint32_t init_param, uint32_t priority);
void sys_log(const char * str);
uint32_t sys_get_pid(void);
uint32_t sys_exit(uint32_t exit_code);
uint32_t sys_mon_create();
uint32_t sys_mon_enter(uint32_t mid);
void sys_mon_exit(uint32_t mid);
void sys_mon_wait(uint32_t mid);
void sys_mon_notify(uint32_t mid);

#endif // __TOAST_H__
