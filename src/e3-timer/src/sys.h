// Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License
#ifndef __SYS_H__
#define __SYS_H__

#define SYS_LED_RED      23
#define SYS_LED_YELLOW   24
#define SYS_LED_GREEN    25

// System call to busy wait
void sys_busywait(unsigned busy);

#endif // __SYS_H__
