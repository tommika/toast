// Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "toast.h"
#include "util.h"
#include "proctl.h"
#include "arm.h"

#define MAX_PROCESS 8
Process process_mem[MAX_PROCESS];

#define PROC_MAGIC 2112

static Process * process_ready = NULL; // ready queue

void proctl_init() {
    for(int pid=0; pid<MAX_PROCESS; pid++) {
        process_mem[pid].flags = 0;
    }
    process_ready = NULL;
}

 void proctl_insert_ready(Process * insert) {
    ASSERT(insert->next==NULL,FC_INVALID_PROC_STATE)
    ASSERT(insert->magic==PROC_MAGIC,FC_INVALID_PROC_MAGIC)
    Process * after = NULL;
    // Find insertion point
    for(Process * t = process_ready;
        t && insert->prio >= t->prio;
        after=t, t=t->next);
    if(!after) {
        // insert at front of queue
        insert->next = process_ready;
        process_ready = insert;
    } else {
        insert->next = after->next;
        after->next = insert;
    }
}

Process * proctl_pop_ready() {
    ASSERT(process_ready!=NULL,FC_EMPTY_QUEUE)
    ASSERT(process_ready->magic==PROC_MAGIC,FC_INVALID_PROC_MAGIC)
    Process * p = process_ready;
    process_ready = p->next;
    p->next = NULL;
    return p;
}

Process * proctl_create(ProcessMainFn entry_point, uint32_t init_param, uint32_t priority) {
    // find a free process struct
    int32_t pid = 0;
    for(; pid<MAX_PROCESS; pid++) {
        if(process_mem[pid].flags==0) {
            break;
        }
    }
    ASSERT(pid!=MAX_PROCESS,FC_OUT_OF_MEM)

    Process * p = &process_mem[pid];
    for(int i=0; i<MAX_REGISTERS;i++) {
        p->registers[i] = 0;
    }
    p->registers[13] = (uint32_t)&(p->stack_mem[STACK_SIZE]);
    p->ps = CPSR_MODE_USR | CPSR_DISABLE_FIQ;
    p->pc = (uint32_t)entry_point;
    p->next = NULL;
    p->pid = pid;
    p->init_param = init_param;
    p->flags = P_INUSE;
    p->prio = priority;
    p->magic = PROC_MAGIC;

    return p;
}
