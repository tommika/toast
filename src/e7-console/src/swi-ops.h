// Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License
#ifndef __SWI_OPS__
#define __SWI_OPS__

// SWI operator values (24-bit)
#define SWI_SET_LED      0x0001
#define SWI_CLOCK_MILLIS 0x0002
#define SWI_FORK         0x0003
#define SWI_MON_CREATE   0x0004
#define SWI_MON_EXIT     0x0005
#define SWI_MON_NOTIFY   0x0006
#define SWI_GET_PID      0x0007

// Blocking operations
#define SWI_BLOCKING     0x8000
#define SWI_EXIT         0x8000
#define SWI_YIELD        0x8001
#define SWI_SLEEP_MILLIS 0x8002
#define SWI_MON_ENTER    0x8003
#define SWI_MON_WAIT     0x8004
#define SWI_LOG          0x8005

#define SWI_MASK         0xFF000000

#endif // __SWI_OPS__
