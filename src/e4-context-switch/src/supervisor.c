// Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License
/* sys.c
 *
 * System-level code that runs in Supervisor mode.
 * 
  */
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "toast.h"
#include "util.h"
#include "gpio.h"
#include "interrupts.h"
#include "proctl.h"

extern void idle(void);
extern void green_blinker(void);
extern void yellow_blinker(void);

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
            busywait(0x80000);
        }
        for(int i=3; i>=0; i--) {
            gpio_write(SYS_LED_BLUE,i==0);
            gpio_write(SYS_LED_RED,i==1);
            gpio_write(SYS_LED_YELLOW,i==2);
            gpio_write(SYS_LED_GREEN,i==3);
            busywait(0x80000);
        }
    }
    // Clear LEDs
    gpio_write(SYS_LED_BLUE,0);
    gpio_write(SYS_LED_RED,0);
    gpio_write(SYS_LED_YELLOW,0);
    gpio_write(SYS_LED_GREEN,0);
    busywait(0x400000);
}

/*! Invoked on system reset
 *
 */
Process * s_init(void) {
    static int rush = 2112;
    ASSERT(rush==2112,FC_ALREADY_INITIALIZED)
    rush = 0;

    gpio_set_func(SYS_LED_BLUE,GPF_OUTPUT);
    gpio_set_func(SYS_LED_RED,GPF_OUTPUT);
    gpio_set_func(SYS_LED_YELLOW,GPF_OUTPUT); 
    gpio_set_func(SYS_LED_GREEN,GPF_OUTPUT);
    
    animate_leds(2);

    proctl_init();

    Process * p;
    p = proctl_create(idle,0,255);
    proctl_insert_ready(p);
    p = proctl_create(green_blinker,0,0);
    proctl_insert_ready(p);
    p = proctl_create(yellow_blinker,0,0);
    proctl_insert_ready(p);

    // Enable timer and timer interrupts
    bool enable_timer = true;
    if(enable_timer) {
        irq_registers->enable_basic_irqs = IRQ_TIMER;
        timer_registers->control = TIMER_CTRL_23BIT 
                                | TIMER_CTRL_PRESCALE_256
                                | TIMER_CTRL_INT_ENABLE 
                                | TIMER_CTRL_ENABLE
                                ;
        timer_registers->reload = 0x80;
        timer_registers->load = 0x80;
    }

    return proctl_pop_ready();
}

/*! Process scheduler
 *
*/
Process * s_schedule(Process * running) {
    ASSERT(running!=NULL,FC_NO_PROCESS)
    if(timer_registers->masked_irq) {
        timer_registers->irq_ack = IRQ_TIMER;
        static bool on = true;
        gpio_write(SYS_LED_BLUE,on);
        on = !on;
        proctl_insert_ready(running);
        running = proctl_pop_ready();
    }
    return running;
}

#define SWI_BUSYWAIT 0
#define SWI_SET_LED  1
#define SWI_YIELD    2

/*! System-call router
 *
 * If there is to be a context-switch, returns a pointer to the process to dispatch.
 * Otherwise, returns NULL.
 * 
 */
Process * s_sys_router(Process * running, int swi_num, const uint32_t * args) {
    ASSERT(running!=NULL,FC_NO_PROCESS)
    Process * dispatch = NULL;
    switch(swi_num) {
    case SWI_SET_LED:
        gpio_write(args[0],args[1]);
        break;
    case SWI_YIELD:
        proctl_insert_ready(running);
        dispatch = proctl_pop_ready();
        break;
    default:
        // invalid
        break;
    }
    return dispatch;
}

void idle() {
    while(1) {
        // ON
        sys_set_led(SYS_LED_RED,1);
        busywait(0x80000);
        // OFF
        sys_set_led(SYS_LED_RED,0);
        busywait(0x800000);
    }
}
