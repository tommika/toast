// Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License
#ifndef __BCM2835_H__
#define __BCM2835_H__

#include <stdint.h>

// See: BCM2835 ARM Peripherals
//  https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf

// 1.2.3 ARM physical addresses
// Physical addresses start at 0x00000000 for RAM.
//
// The bus addresses for peripherals are set up to map onto the peripheral 
// bus address range starting at 0x7E000000.

// The base bus address on  the Raspberry Pi 1 is actually 0x20000000
#define BASE_BUS_ADDR 0x20000000UL  // RPi-1

// BCM2835 ARM Peripherals: Section 7, Interrupts
#define IRQ_REGISTERS_OFFSET 0xB200UL
volatile typedef struct {
    uint32_t irq_basic_pending;  
    uint32_t irq_pending_1;
    uint32_t irq_pending_2;
    uint32_t fiq_control;
    uint32_t enable_irqs_1;
    uint32_t enable_irqs_2;
    uint32_t enable_basic_irqs;
    uint32_t disable_irqs_1;
    uint32_t disable_irqs_2;
    uint32_t disable_basic_irqs;
} IRQ_Registers;
volatile IRQ_Registers * irq_registers = (IRQ_Registers*)(BASE_BUS_ADDR + IRQ_REGISTERS_OFFSET);

#define IRQ_TIMER            0b00000001
#define IRQ_MAILBOX          0b00000010
#define IRQ_DOORBELL_0       0b00000100
#define IRQ_DOORBELL_1       0b00001000
#define IRQ_GPU_0_HALTED     0b00010000
#define IRQ_GPU_1_HALTED     0b00100000
#define IRQ_ILLEGAL_ACCESS_1 0b01000000
#define IRQ_ILLEGAL_ACCESS_2 0b10000000

// BCM2835 ARM Peripherals: Section 12, SYstem Timer
#define SYSTEM_TIMER_OFFSET 0x3000UL
volatile typedef struct {
    uint32_t cs;   // System Timer Control/Status
    uint32_t clo;  // System Timer Counter Lower 32 bits
    uint32_t chi;  // System Timer Counter Higher 32 bits
    uint32_t c0;   // System Timer Compare 0
    uint32_t c1;   // System Timer Compare 1
    uint32_t c2;   // System Timer Compare 2
    uint32_t c3;   // System Timer Compare 3
} System_Timer_Registers;
volatile System_Timer_Registers * system_timer_registers = (System_Timer_Registers*)(BASE_BUS_ADDR + SYSTEM_TIMER_OFFSET);

// BCM2835 ARM Peripherals: Section 14, ARM Timer (ARM Side)
#define TIMER_REGISTERS_OFFSET 0xB400UL
volatile typedef struct {
    uint32_t load;
    uint32_t value;
    uint32_t control;
    uint32_t irq_ack;
    uint32_t raw_irq;
    uint32_t masked_irq;
    uint32_t reload;
    uint32_t pre_divider;
    uint32_t free_running_counter;
} Timer_Registers;
volatile Timer_Registers * timer_registers = (Timer_Registers*)(BASE_BUS_ADDR + TIMER_REGISTERS_OFFSET);

// Timer control register bits
// BCM2835 p. 197 
                                             // bit 0 not used
#define TIMER_CTRL_23BIT         0b00000010  // Enable 24-bit counter; default is 16-bit counter
#define TIMER_CTRL_PRESCALE_1    0b00000000  // clock / 1 (no pre-scale)
#define TIMER_CTRL_PRESCALE_16   0b00000100  // clock / 16
#define TIMER_CTRL_PRESCALE_256  0b00001000  // clock / 256
                                             // bit 4 not used
#define TIMER_CTRL_INT_ENABLE    0b00100000  // Enable timer interrupts
                                             // bit 6 not used
#define TIMER_CTRL_ENABLE        0b10000000  // Enable timer

#endif // __BCM2835_H__