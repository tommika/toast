// Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License
#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__

#include <stdint.h>


typedef uint32_t (*TimerCallbackFn)(uint32_t);

void timer_set(uint32_t val, TimerCallbackFn fn);

#endif // __INTERRUPTS_H__