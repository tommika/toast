// Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License
#include "arm.h"
#include "swi-ops.h"

.section .init
.globl _start
_start:
    @ Exception vector
    ldr pc,reset_addr
    ldr pc,undef_addr
    ldr pc,swi_addr
    ldr pc,prefetch_addr
    ldr pc,abort_addr
    ldr pc,reserved_addr
    ldr pc,irq_addr
    ldr pc,fiq_addr    

reset_addr:      .word reset_handler
undef_addr:      .word hang
swi_addr:        .word swi_handler
prefetch_addr:   .word hang
abort_addr:      .word hang
reserved_addr:   .word hang
irq_addr:        .word irq_handler
fiq_addr:        .word hang 

#define ADDR_ORIGIN 0x8000     

@ The reset handler is invoked on initial boot (and for system resets.)
@ This will initialize the supervisor, queue-up initial processes,
@ and dispatch the root process.
reset_handler:
    @ Copy exception vector from 0x8000 to 0x0000  
    mov     r0, #0x0000         @ dest
    mov     r1, #ADDR_ORIGIN    @ source (exception vector)
    mov     r2, #16             @ copy 16 words
    bl      copy_words
    @ Initialize the Supervisor
    mov     r0, #(CPSR_MODE_SVC | CPSR_DISABLE_IRQ | CPSR_DISABLE_FIQ )
    msr     cpsr, r0            @ Switch to Supervisor-mode
    mov     sp, #ADDR_ORIGIN    @ Set the stack to be used for Supervisor-mode
    bl      s_init              @ Call the supervisor initialization function.
    @ r0 now point to Process to be dispatched
    mov     r1, #(CPSR_MODE_IRQ | CPSR_DISABLE_IRQ | CPSR_DISABLE_FIQ )
    msr     cpsr, r1            @ Switch to IRQ-Mode
    b       dispatch            @ Dispatch
swi_handler:                    @ Software interrupt (SWI)
                                @ Will be in supervisor mode
    mov     sp, #ADDR_ORIGIN    @ Stack for interrupt handlers
    stmfd   sp!,{r0-r12,lr}     @ Save registers and return address
                                @ Get pointer to running Process
                                @ This is stored in sp_irq: sp_irq==&running->registers[0]
                                @ Nneed to change modes to get sp_irq on ARMv6
    mrs     r1, cpsr            @ Save current cpsr
    mov     r0, #(CPSR_MODE_IRQ | CPSR_DISABLE_IRQ | CPSR_DISABLE_FIQ )
    msr     cpsr, r0            @ Switch to IRQ-mode
    mov     r0, sp              @ Get irq_sp
    msr     cpsr, r1            @ Switch back to saved (Supervisor) mode
                                @ r0 == &running->registers[0]
    ldr     r1, [lr,#-4]        @ SWI instruction is located in the word before the return address (lr)                
    bic     r1, r1, #SWI_MASK   @ Mask-off the SWI op-code
    cmp     r1, #SWI_BLOCKING   @ See if it's a potentially blocking SWI
    blt     _router             @ Don't save additional process state unless it's a potentially blocking SWI
    mrs     r3, spsr            @ Get User-mode process status register
    stmdb   r0, {r3,lr}         @ Save User-mode spsr and lr (return address) to Process struct
    add     r2, r0, #16         @ r2 == process->registers[4]
    stm     r2, {r4-r14}^       @ Save User-mode process registers to Process struct
_router:
    sub     r0, r0, #8          @ Update r0 to point to start of Process struct
    mov     r2, sp              @ Pass input parameters to router
    bl      s_sys_router        @ Brach to the router
                                @ Returns next process in r0, or no switch if r0==0
    cmp     r0, #0              @ See what it is
    ldmeqfd sp!, {r0-r12,pc}^   @ Return to caller if r0==0
                                @ r0 points to Process to be dispatched
    mov     r1, sp
    mov     r3, #(CPSR_MODE_IRQ | CPSR_DISABLE_IRQ | CPSR_DISABLE_FIQ )
    msr     cpsr, r3            @ Change to IRQ-mode
                                @ sp == &running->registers[0]
                                @ r1 is pointing registers passed to router (return values)
                                @ r0 is point at Process to be dispatched
    ldr     r4,[r1]             @ Save the SWI return value to running->registers[0]
    str     r4,[sp]
    b       dispatch            @ Disaptch process pointed to by r0

irq_handler:                    @ IRQ Handler
                                @ sp_irq == &running->registers[0]
    stm     sp, {r0-lr}^        @ Save User-mode process registers
    mrs     r0, spsr            @ Get user-mode process status register (spsr)
    sub     lr, lr, #4          @ Get user-mode program counter (pc)
    stmdb   sp, {r0, lr}        @ Save User-mode spsr and pc to Process struct
    sub     r0, sp, #8          @ Adjust to point to front on Process struct
    mov     sp, #ADDR_ORIGIN    @ Set-up stack used by scheduler
    bl      s_schedule          @ Call scheduler
                                @ Returns pointer to next Process in r0
                                @ McDonalds Drive-Thru
dispatch:                       @ dispatch assumes that:
                                @ - processor is in IRQ-mode
                                @ - r0 is pointing at process to dispatch
    add     sp, r0, #8          @ Set sp_irq to &process->registers[0]
    ldmdb   sp, {r0, lr}        @ Get the saved process status and pc (program counter)
    msr     spsr_cxsf, r0       @ Restore the process status
    ldm     sp, {r0-lr}^        @ Restore User-mode process registers and pc
    movs    pc, lr              @ Switch to User-mode and resume execution of process

@ copy_words(r0=dest,r1=src,r2=count):
@ Copy a non-overallaping range of memory
copy_words:
    cmp     r2, #0
    beq     done_cr
    ldr     r3, [r1], #4        @ r3 = *(r1++)
    str     r3, [r0], #4        @ *(r0++) = r3
    sub     r2, r2, #1
    b       copy_words 
done_cr:
    bx      lr

hang:
    mov     r0, #1
    b       panic

.global busywait
busywait:
    subs    r0, r0, #1
    bne     busywait
    bx      lr

.global sys_set_led
sys_set_led:
    push {lr}
    swi SWI_SET_LED
    pop {pc}

.global sys_clock_millis
sys_clock_millis:
    push {lr}
    swi SWI_CLOCK_MILLIS
    pop {pc}

.global sys_fork
sys_fork:
    push {lr}
    swi SWI_FORK
    pop {pc}

.global sys_yield
sys_yield:
    push {lr}
    swi SWI_YIELD
    pop {pc}

.global sys_sleep_millis
sys_sleep_millis:
    push {lr}
    swi SWI_SLEEP_MILLIS
    pop {pc}

.global sys_mon_create
sys_mon_create:
    push {lr}
    swi SWI_MON_CREATE
    pop {pc}

.global sys_mon_enter
sys_mon_enter:
    push {lr}
    swi SWI_MON_ENTER
    pop {pc}

.global sys_mon_exit
sys_mon_exit:
    push {lr}
    swi SWI_MON_EXIT
    pop {pc}

.global sys_mon_wait
sys_mon_wait:
    push {lr}
    swi SWI_MON_WAIT
    pop {pc}

.global sys_mon_notify
sys_mon_notify:
    push {lr}
    swi SWI_MON_NOTIFY
    pop {pc}

.global sys_exit
sys_exit:
    push {lr}
    swi SWI_EXIT
    pop {pc}

.global sys_log
sys_log:
    push {lr}
    swi SWI_LOG
    pop {pc}

.global sys_get_pid
sys_get_pid:
    push {lr}
    swi SWI_GET_PID
    pop {pc}

