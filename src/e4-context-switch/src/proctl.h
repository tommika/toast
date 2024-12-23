// Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License
#ifndef __PROCTL_H__
#define __PROCTL_H__
#include <stdint.h>

typedef void (*ProcessMainFn)(void);

#define STACK_SIZE 0x1000

// Process flags
#define P_INUSE 0b00000001 // process control struct is in use
#define P_INIT  0b00000010 // process has been initialized

#define MAX_REGISTERS 15

typedef struct Process_S {
    uint32_t ps;                       // Saved Process Status
    uint32_t pc;                       // Saved Program Counter
    uint32_t registers[MAX_REGISTERS]; // Saved general registers
    struct Process_S * next;           // Next process in (some) queue
    uint32_t flags;                    // Process flags
    uint32_t pid;                      // Process identifier
    uint32_t init_param;               // Process init parameter
    uint32_t prio;                     // Process priority
    uint32_t stack_mem[STACK_SIZE];    // Process stack memory. This is a hack
                                       // but sufficient for now.
    uint32_t magic;
} Process;

void proctl_init();
void proctl_insert_ready(Process * insert);
Process * proctl_pop_ready();
Process * proctl_create(ProcessMainFn entry_point, uint32_t init_param, uint32_t priority);

#endif // __PROCTL_H__