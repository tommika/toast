
@ Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License
.equ    CPSR_MODE_USER,         0x10
.equ    CPSR_MODE_FIQ,          0x11
.equ    CPSR_MODE_IRQ,          0x12
.equ    CPSR_MODE_SVR,          0x13
.equ    CPSR_MODE_ABORT,        0x17
.equ    CPSR_MODE_HYPERVISOR,   0x1A
.equ    CPSR_MODE_UNDEFINED,    0x1B
.equ    CPSR_MODE_SYSTEM,       0x1F
.equ    CPSR_MODE_MASK,         0x1F // Used to mask the Mode bit in the CPSR

// See ARM section A2.5 (Program status registers)
.equ    CPSR_IRQ_INHIBIT,       0x80
.equ    CPSR_FIQ_INHIBIT,       0x40
.equ    CPSR_THUMB,             0x20

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

reset_addr:      .word reset
undef_addr:      .word hang
swi_addr:        .word swi_handler
prefetch_addr:   .word hang
abort_addr:      .word hang
reserved_addr:   .word hang
irq_addr:        .word c_irq_handler
fiq_addr:        .word hang 

reset:
    @ Copy exception vector from 0x8000 to 0x0000  
    mov r0, #0x0000  @ dest
    mov r1, #0x8000  @ source (exception vector)
    mov r2, #16      @ copy 16 words
    bl copy_range

    @ Set Interrupt mode stack pointer
    mov r0, #(CPSR_MODE_IRQ | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT )
    msr cpsr_c, r0
    mov sp, #0x7000

    @ Set Supervisor mode stack pointer
    mov r0, #(CPSR_MODE_SVR | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT )
    msr cpsr_c, r0
    mov sp, #0x8000

    @ Branch to C code
    bl  c_reset_handler

    @ If C code returns ...
    b hang

swi_handler:
    @ save registers and return address
    stmfd sp!,{r0-r12,lr}

    @ call C handler
    ldr   r0, [lr,#-4]
    bic   r0, r0, #0xff000000
    mov   r1, sp
    bl    c_swi_handler

    @ restore registers, return address 
    ldmfd sp!, {r0-r12,pc}^

hang:
    b .

# copy_words(dest,src,count)
#    copy non-overallaping range of memory
copy_range:
    cmp r2, #0
    beq done_cr
    ldr r3, [r1], #4  @r3 = *(r1++)
    str r3, [r0], #4  @*(r0++) = r3
    sub r2, r2, #1
    b   copy_range 
done_cr:
    bx lr


# dispatch(address)
.section .text
.global dispatch
dispatch:
    mov    lr, r0
    mov    sp, r1
    mrs    r0, cpsr            @ load CPSR into R0
    bic    r0, r0, #0x1F       @ clear mode field
    bic    r0, r0, #0x80       @ enable interrupts
    orr    r0, r0, #0x10       @ user mode code
    msr    spsr, r0            @ store modified CPSR into SPSR
    movs   pc, lr              @ context switch and branch

@ System calls
.global sys_busywait
sys_busywait:
    push {lr}
    swi 0x0
    pop {pc}
