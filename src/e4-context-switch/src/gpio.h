// Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License
#ifndef __GPIO_H__
#define __GPIO_H__

// Valid GPIO lines are [0,GPIO_MAX)
#define GPIO_MAX 54

// Valid GP function/modes [0,GPF_MAX)
#define GPF_INPUT  0
#define GPF_OUTPUT 1
#define GPF_MAX    8

int gpio_set_func(unsigned int gpio, unsigned func);
int gpio_write(unsigned int gpio, unsigned int level);
int enable_activity_led();

#endif // __GPIO_H__
