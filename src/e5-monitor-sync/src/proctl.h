#ifndef __PROCTL_H__
#define __PROCTL_H__
#include <stdint.h>

// Process flags
#define P_ALLOCATED  0b00000001 // Process control struct is in use
#define P_TERMINATED 0b00000010 // Process has terminated

#define MAX_REGISTERS 15
struct Monitor_S;
struct Process_S;

typedef struct Queue_S {
    struct Process_S * head;
} Queue;

#define STACK_SIZE 0x10
#define PID_NONE ((uint32_t)(-1))

typedef struct Process_S {
    uint32_t ps;                       // Saved Process Status
    uint32_t pc;                       // Saved Program Counter
    uint32_t registers[MAX_REGISTERS]; // Saved general registers
    uint32_t pid;                      // Process identifier
    uint32_t parent_pid;               // Parent process identifier
    uint32_t flags;                    // Process flags
    uint32_t sched_prio;               // Process scheduling priority
    uint32_t exit_code;
    uint32_t stack_magic;
    uint32_t stack_mem[STACK_SIZE];    // Process stack memory. This is a hack
                                       // but sufficient for now.
    uint32_t magic;
    struct Process_S * q_next;         // Next process in (some) queue (or NULL if process is running)
    union {
        uint32_t q_prio_uint32;        // Priority on (some) queue (32-bit)
        uint64_t q_prio_uint64;        // Priority on (some) queue (64-bit)
    };
} Process;

#define PROC_MAGIC  2112
#define STACK_MAGIC 0x42424242

/*! Initialize */
void p_init();
/*! Create a new process */
Process * p_create(const Process * parent, uint32_t entry_point, uint32_t init_param, uint32_t priority);
/*! Insert the given process into the ready queue */
void p_ready(Process * insert);
/*! Pop a process from the ready queue. Will panic if the queue is empty */
Process * p_pop_ready();
/*! Terminate the given process */
void p_terminate(Process * p, uint32_t exit_code);

/*! Put the given process to sleep for some time */
void p_sleep(Process * running, uint64_t sleep_until);
/*! Move processes from the sleep queue to the ready queue  */
void p_rouse(uint64_t clock);

// Monitor flags
#define M_ALLOCATED 0b00000001  // Monitor is in use
#define M_OCCUPIED  0b00000010  // Monitor is occupied

typedef struct Monitor_S {
    struct Queue_S entry_q;     // Entrance queue; processes waiting to enter monitor
    struct Queue_S cond_q;      // Condition queue; processes waiting for notification
    struct Process_S * p;       // Process currently occupying the monitor
    uint32_t mid;               // Monitor identifier
    uint32_t flags;             // Monitor flags (see above)
} Monitor;

#define MID_NONE ((uint32_t)(-1))

#define M_OK             0
#define M_BLOCKED        1
#define M_ILLEGAL_ARG   -1
#define M_ILLEGAL_STATE -2

uint32_t m_create();
int m_enter(Process * p, uint32_t mid);
int m_exit(Process * p, uint32_t mid);
int m_wait(Process * p, uint32_t mid);
int m_notify(Process * p, uint32_t mid);

#endif // __PROCTL_H__