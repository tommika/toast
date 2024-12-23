// Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License
#include <stdint.h>
#include <stdbool.h>

#include "proctl.h"
#include "arm.h"
#include "bcm2835.h"
#include "str.h"
#include "assert.h"

uint32_t _proc_main(uint32_t entrypoint, uint32_t init_param);

#define MAX_MONITOR 4
struct Monitor_S monitor_mem[MAX_MONITOR];

#define MAX_PROCESS 8
struct Process_S process_mem[MAX_PROCESS];

static Queue ready_q; // ready queue
static Queue sleep_q; // sleep queue

inline static void q_init(Queue * queue) {
    queue->head = NULL;
}

/*! 32-bit priority value queue */
inline static void q_insert_uint32(Queue * queue, Process * insert, uint32_t q_prio) {
    ASSERT(insert->magic==PROC_MAGIC,FC_INVALID_PROC_MAGIC)
    ASSERT(insert->q_next==NULL,FC_INVALID_PROC_STATE)
    insert->q_prio_uint32 = q_prio;
    // Find insertion point
    Process * after = NULL;
    for(Process * t = queue->head;
        t && q_prio >= t->q_prio_uint32;
        after=t, t=t->q_next);
    if(!after) {
        // insert at front of queue
        insert->q_next = queue->head;
        queue->head = insert;
    } else {
        insert->q_next = after->q_next;
        after->q_next = insert;
    }
}

/*! 64-bit priority value queue */
inline static void q_insert_uint64(Queue * queue, Process * insert, uint64_t q_prio) {
    ASSERT(insert->magic==PROC_MAGIC,FC_INVALID_PROC_MAGIC)
    ASSERT(insert->q_next==NULL,FC_INVALID_PROC_STATE)
    insert->q_prio_uint64 = q_prio;
    // Find insertion point
    Process * after = NULL;
    for(Process * t = queue->head;
        t && q_prio >= t->q_prio_uint64;
        after=t, t=t->q_next);
    if(!after) {
        // insert at front of queue
        insert->q_next = queue->head;
        queue->head = insert;
    } else {
        insert->q_next = after->q_next;
        after->q_next = insert;
    }
}

inline static Process * q_pop(Queue * queue) {
    Process * popped = queue->head;;
    if(popped) {
        ASSERT(popped->magic==PROC_MAGIC,FC_INVALID_PROC_MAGIC)
        queue->head = popped->q_next;
        popped->q_next = NULL;
    }
    return popped;
}

void p_init() {
    for(int pid=0; pid<MAX_PROCESS; pid++) {
        process_mem[pid].flags = 0;
    }
    q_init(&ready_q);
    q_init(&sleep_q);

    for(int mid=0; mid<MAX_MONITOR; mid++) {
        monitor_mem[mid].flags = 0;
    }
}

Process * p_create(const Process * parent, uint32_t entry_point, uint32_t init_param, uint32_t priority) {
    // find a free process struct
    int32_t pid = 0;
    for(; pid<MAX_PROCESS; pid++) {
        if(process_mem[pid].flags==0) {
            break;
        }
    }
    ASSERT(pid<MAX_PROCESS,FC_OUT_OF_PROC)
    Process * p = &process_mem[pid];
    for(int i=0; i<MAX_REGISTERS;i++) {
        p->registers[i] = 0;
    }
    p->registers[R_R0] = (uint32_t)entry_point;
    p->registers[R_R1] = init_param;
    p->registers[R_SP] = (uint32_t)&(p->stack_mem[STACK_SIZE]);
    p->ps = CPSR_MODE_USR | CPSR_DISABLE_FIQ;
    p->pc = (uint32_t)_proc_main;
    p->pid = pid;
    if(parent) {
        p->parent_pid = parent->pid;
    } else {
        p->parent_pid = (uint32_t)(-1);
    }
    p->flags = P_ALLOCATED;
    p->sched_prio = priority;
    p->stack_magic = STACK_MAGIC;
    p->magic = PROC_MAGIC;
    p->q_next = NULL;
    char buff[32];
    uart_puts("creating process: pid=");
    uart_puts(itoa(p->pid,buff,10));
    uart_puts("\r\n");
    for(int r=0;r<MAX_REGISTERS;r++) {
        if(r!=0) {
            uart_puts(" ");
        }
        uart_puts("r");
        uart_puts(itoa(r,buff,10));
        uart_puts("=");
        uart_puts(itoa(p->registers[r],buff,16));
    }
    uart_puts("\r\n");
    return p;
}

void p_ready(Process * insert) {
    q_insert_uint32(&ready_q, insert, insert->sched_prio);
}

Process * p_pop_ready() {
    Process * p = q_pop(&ready_q);
    ASSERT(p,FC_EMPTY_QUEUE)
    return p;
}

void p_terminate(Process * running, uint32_t exit_code) {
    ASSERT(running->magic==PROC_MAGIC,FC_INVALID_PROC_MAGIC)
    ASSERT(running->q_next==NULL,FC_INVALID_PROC_STATE)
    uart_puts("Process has terminated: pid=");
    uart_putn(running->pid);
    uart_puts(", exit_code=");
    uart_putn(exit_code);
    uart_puts("\r\n");

    running->exit_code = exit_code;
    running->flags |= P_TERMINATED;
}

/*! Insert the given process into the sleep queue.
 *
 */
 void p_sleep(Process * running, uint64_t sleep_until) {
    q_insert_uint64(&sleep_q, running, sleep_until);
}

/*! Move processes from the sleep queue to the ready queue
 *
 */
void p_rouse(uint64_t clock) {
    while(sleep_q.head && sleep_q.head->q_prio_uint64 <= clock) {
        Process * p = q_pop(&sleep_q);
        q_insert_uint32(&ready_q,p,p->sched_prio);
    }
}

uint32_t m_create() {
    // find a free monitor struct
    int32_t mid = 0;
    for(; mid<MAX_MONITOR; mid++) {
        if(monitor_mem[mid].flags==0) {
            break;
        }
    }
    ASSERT(mid<MAX_MONITOR,FC_OUT_OF_MON)
    Monitor * m = &monitor_mem[mid];
    q_init(&m->entry_q);
    q_init(&m->cond_q);
    m->p = NULL;
    m->mid = mid;
    m->flags = M_ALLOCATED;
    return m->mid;
}

Monitor * m_lookup(uint32_t mid) {
    if(mid<0 || mid>=MAX_MONITOR) {
        return NULL;
    }
    Monitor * m = &monitor_mem[mid];
    return (m->flags & M_ALLOCATED) ? m : NULL;
}

static void next_ready(Monitor * m) {
    Process * ready = q_pop(&m->entry_q);
    if(!ready) {
        // No one waiting to enter
        m->p = NULL;
        m->flags &= ~M_OCCUPIED;
    } else {
        m->p = ready;
        p_ready(ready);
    }
}

int m_enter(Process * p, uint32_t mid) {
    Monitor * m = m_lookup(mid);
    if(!m) {
        ASSERT(false,FC_ILLEGAL_ARG)
        return M_ILLEGAL_ARG;
    }
    if(!(m->flags & M_OCCUPIED)) {
        ASSERT(m->p == NULL,FC_INVALID_MON_STATE)
        m->flags |= M_OCCUPIED;
        m->p = p;
        return M_OK;
    }
    ASSERT(m->p,FC_INVALID_MON_STATE)

    // There's already a process in the monitor (thou shall not pass)
    // Add the process to entry queue
    // NOTE: currently using process scheduling priority as the monitor priority
    q_insert_uint32(&m->entry_q,p,p->sched_prio);
    return M_BLOCKED;
}

int m_exit(Process * p, uint32_t mid) {
    Monitor * m = m_lookup(mid);
    if(!m) {
        // TODO: set error code
        ASSERT(false,FC_ILLEGAL_ARG)
        return M_ILLEGAL_ARG;
    }
    if(m->p!=p) {
        // Process is not associated with this monitor
        // TODO: set error code
        ASSERT(false,FC_ILLEGAL_STATE)
        return M_ILLEGAL_STATE;
    }
    ASSERT(m->p==p,FC_WRONG_PROCESS)
    next_ready(m);
    return M_OK;
}

int m_wait(Process * p, uint32_t mid) {
    Monitor * m = m_lookup(mid);
    if(!m) {
        ASSERT(false,FC_ILLEGAL_ARG)
        return M_ILLEGAL_ARG;
    }
    if(m->p!=p) {
        // Process is not associated with this monitor
        // TODO: set error code
        ASSERT(false,FC_ILLEGAL_STATE)
        return M_ILLEGAL_STATE;
    }
    // insert into condition queue
    // NOTE: currently using process priority as cond priority
    q_insert_uint32(&m->cond_q,p,p->sched_prio);
    next_ready(m);
    return M_BLOCKED; 
}

int m_notify(Process * p, uint32_t mid) {
    Monitor * m = m_lookup(mid);
    if(!m) {
        ASSERT(false,FC_ILLEGAL_ARG)
        return M_ILLEGAL_ARG;
    }
    if(m->p!=p) {
        // Process is not associated with this monitor
        // TODO: set error code
        ASSERT(false,FC_ILLEGAL_STATE)
        return M_ILLEGAL_STATE;
    }
    Process * waiting = q_pop(&m->cond_q);
    if(waiting) {
        // Wake-up a the waiting process
        // NOTE: currently using process priority as entry priority
        q_insert_uint32(&m->entry_q,waiting,waiting->sched_prio);
    }
    return M_OK;
}
