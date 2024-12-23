// Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License
/* supervisor.c
 *
 * System-level code that runs in Supervisor mode.
 * 
 */
#include <stdint.h>
#include <stdbool.h>
#include "toast.h"
#include "bcm2835.h"
#include "proctl.h"
#include "swi-ops.h"
#include "str.h"
#include "assert.h"

#define NOINLINE __attribute__((noinline))

void idle_proc(uint32_t init_param);
void root_proc(uint32_t init_param);

extern uint32_t app_main(uint32_t init_param);

/*! A little LED animation
 *
 * \param iters Number of iterations in the animation
 *
 */
NOINLINE void animate_leds(int iters) {
    while(iters--) {
        for(int i=0; i<4; i++) {
            gpio_write(SYS_LED_BLUE,i==0);
            gpio_write(SYS_LED_RED,i==1);
            gpio_write(SYS_LED_YELLOW,i==2);
            gpio_write(SYS_LED_GREEN,i==3);
            busy_wait_millis(50);
        }
        for(int i=3; i>=0; i--) {
            gpio_write(SYS_LED_BLUE,i==0);
            gpio_write(SYS_LED_RED,i==1);
            gpio_write(SYS_LED_YELLOW,i==2);
            gpio_write(SYS_LED_GREEN,i==3);
            busy_wait_millis(50);
        }
    }
    // Clear LEDs
    gpio_write(SYS_LED_BLUE,0);
    gpio_write(SYS_LED_RED,0);
    gpio_write(SYS_LED_YELLOW,0);
    gpio_write(SYS_LED_GREEN,0);
    busy_wait_millis(1000);
}

/*! Invoked on system reset
 *
 * Returns a pointer to the Process the initial Process to be dispatched.
 */
Process * s_init(void) {
    static int rush = 2112;
    ASSERT(rush==2112,FC_ALREADY_INITIALIZED)
    rush = 0;

    gpio_set_func(SYS_LED_BLUE,GPF_OUTPUT);
    gpio_set_func(SYS_LED_RED,GPF_OUTPUT);
    gpio_set_func(SYS_LED_YELLOW,GPF_OUTPUT); 
    gpio_set_func(SYS_LED_GREEN,GPF_OUTPUT);
    
    animate_leds(3);

    uart_init(BAUD_115200);
    uart_puts("\033c"); // clear screen
    uart_puts("\033[32;1mTOAST\033[0m is starting up\r\n");

    p_init();

    p_ready(p_create(NULL,(uint32_t)root_proc,0,0));   // root process
    p_ready(p_create(NULL,(uint32_t)idle_proc,0,255)); // idle process

    // Enable timer and timer interrupts for time slicing.

    unsigned int quantum = 0x40;
    if(quantum>0) {
        // timer freq = sys_clk / (prescale+1)
        irq_registers->enable_basic_irqs = IRQ_TIMER;
        timer_registers->control = TIMER_CTRL_23BIT 
                                 | TIMER_CTRL_PRESCALE_1
                                 | TIMER_CTRL_INT_ENABLE 
                                 | TIMER_CTRL_ENABLE
                                 ;
        timer_registers->reload = quantum;
        timer_registers->load = quantum;
    }

    uart_puts("Dispatching root process\r\n");

    return p_pop_ready();
}

/*! Process scheduler
 *
*/
Process * s_schedule(Process * running) {
    ASSERT(running!=NULL,FC_NO_PROCESS)
    ASSERT(running->magic==PROC_MAGIC,FC_INVALID_PROC_MAGIC)
    ASSERT(running->stack_magic==STACK_MAGIC,FC_STACK_OVERFLOW)
    p_rouse(system_timer());
    if(timer_registers->masked_irq) {
        timer_registers->irq_ack = IRQ_TIMER;
        p_ready(running);
        running = p_pop_ready();

        // Pulsing LED
        #define DELAY 50
        static unsigned char step = 0;
        static unsigned char val = 0;
        static unsigned int n = DELAY;
        static char dir = 1;
        step--;
        if(val>0 && val >= step) {
            gpio_write(SYS_LED_BLUE,true);
        } else {
            gpio_write(SYS_LED_BLUE,false);
        }
        n--;
        if(n==0) {
            n = DELAY;
            val += dir;
            if(val==0 || val==255) {
                dir = -dir;
            }
        }
    }
    return running;
}

/*! System-call router
 *
 */
Process * s_sys_router(Process * running, int swi_num, uint32_t * args) {
    ASSERT(running!=NULL,FC_NO_PROCESS)
    ASSERT(running->magic==PROC_MAGIC,FC_INVALID_PROC_MAGIC)
    ASSERT(running->stack_magic==STACK_MAGIC,FC_STACK_OVERFLOW)
    Process * dispatch = NULL;
    switch(swi_num) {
    default:
        // invalid
        uart_puts("WARNING: Invalid SWI: ");
        uart_putn(swi_num);
        uart_puts("\n");
        break;
    case SWI_GET_PID:
        args[0] = running->pid;
        break;        
    case SWI_SET_LED:
        gpio_write(args[0],args[1]);
        break;
    case SWI_CLOCK_MILLIS:
        args[0] = system_timer()/1000;
        break;
    case SWI_FORK: {
        Process * p = p_create(
            running,
            args[0],
            args[1],
            args[2]);
        p_ready(p);
        // Return the new pid to the calling process
        running->registers[0] = p->pid;
        break;
        }
    case SWI_MON_CREATE:
        args[0] = m_create();
        break;
    case SWI_MON_EXIT:
        args[0] = m_exit(running,args[0]);
        break;
    case SWI_MON_NOTIFY:
        args[0] = m_notify(running,args[0]);
        break;
    // (Potentially) blocking SWIs
    case SWI_EXIT:
        p_terminate(running,args[0]);
        dispatch = p_pop_ready();
        break;
    case SWI_YIELD:
        p_ready(running);
        dispatch = p_pop_ready();
        break;
    case SWI_SLEEP_MILLIS:
        p_sleep(running, system_timer() + ((uint64_t)1000 * args[0]));
        dispatch = p_pop_ready();
        break;
    case SWI_MON_ENTER:
        args[0] = m_enter(running,args[0]);
        if(args[0]==M_BLOCKED) {
            dispatch = p_pop_ready();
        }
        break;
    case SWI_MON_WAIT:
        args[0] = m_wait(running,args[0]);
        if(args[0]==M_BLOCKED) {
            dispatch = p_pop_ready();
        }
        break;
    case SWI_LOG: {
        char buff[8];
        uart_puts("\033[33;1m");
        uart_puts("pid=");
        uart_puts(itoa(running->pid,buff,10));
        uart_puts(": ");
        uart_puts("\033[0m");
        uart_puts((const char *)args[0]);
        uart_puts("\r\n");
        }
        break;
    }
    return dispatch;
}

// Low priority idle process
void idle_proc(uint32_t init_param) {
    while(1);
}

uint32_t countdown_proc(uint32_t init_param) {
    int n = init_param;
    sys_log("countdown_proc starting");
    while(n>0) {
        sys_log(".");
        sys_sleep_millis(100);
        n--;
    }
    sys_log("countdown_proc exiting");
    return 0;
}

void root_proc(uint32_t init_param) {
    sys_log("root_proc is forking app_main");
    sys_fork(app_main,0,0);
    while(1) {
        int c = uart_getc();
        uart_putc(c);
        if(c=='\n') {
            uart_puts("\r\n*newline\r\n");
        }
        else if(c=='\r') {
            uart_puts("\r\n*linefeed\r\n");
        }
        else if(c>='1' && c<='9') {
            sys_log("root_proc is forking a child");
            sys_fork((ProcessMainFn)countdown_proc,(uint32_t)(c-'0'),0);
        }
    }
}
