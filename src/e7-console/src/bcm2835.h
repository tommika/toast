// Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License
#ifndef __BCM2835_H__
#define __BCM2835_H__

#include <stdint.h>
#ifndef NULL
#define NULL ((void*)0)
#endif

// See: BCM2835 ARM Peripherals
//  https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf

// 1.2.3 ARM physical addresses
// Physical addresses start at 0x00000000 for RAM.
//
// The bus addresses for peripherals are set up to map onto the peripheral 
// bus address range starting at 0x7E000000.

// The base bus address on the Raspberry Pi 1 is actually 0x20000000
#define BASE_BUS_ADDR 0x20000000UL  // RPi-1

// BCM2835 ARM Peripherals: Section 7, Interrupts
#define IRQ_REGISTERS_OFFSET 0x0000B200UL
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

#define IRQ_TIMER            0b00000001
#define IRQ_MAILBOX          0b00000010
#define IRQ_DOORBELL_0       0b00000100
#define IRQ_DOORBELL_1       0b00001000
#define IRQ_GPU_0_HALTED     0b00010000
#define IRQ_GPU_1_HALTED     0b00100000
#define IRQ_ILLEGAL_ACCESS_1 0b01000000
#define IRQ_ILLEGAL_ACCESS_2 0b10000000

// BCM2835 ARM Peripherals: Section 12, SYstem Timer
#define SYSTEM_TIMER_OFFSET 0x00003000UL
volatile typedef struct {
    uint32_t cs;   // System Timer Control/Status
    uint32_t clo;  // System Timer Counter Lower 32 bits
    uint32_t chi;  // System Timer Counter Higher 32 bits
    uint32_t c0;   // System Timer Compare 0
    uint32_t c1;   // System Timer Compare 1
    uint32_t c2;   // System Timer Compare 2
    uint32_t c3;   // System Timer Compare 3
} System_Timer_Registers;

// BCM2835 ARM Peripherals: Section 14, ARM Timer (ARM Side)
#define TIMER_REGISTERS_OFFSET 0x0000B400UL
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

// GPIO Registers - GPIO has 41 32-bit registers
#define GPIO_REGISTERS_OFFSET 0x00200000UL
volatile typedef struct {
    uint32_t fsel[6];   // [r/w] function select 0-5
    uint32_t resv1;     // reserved
    uint32_t set[2];    // [w] set output
    uint32_t resv2;     // reserved
    uint32_t clr[2];    // [w] clear output
    uint32_t resv3;     // reserved
    uint32_t lev[2];    // [r] level
    uint32_t resv4;     // reserved
    uint32_t eds[2];    // [r/w] event detect status
    uint32_t resv5;     // reserved
    uint32_t eren[2];   // [r/w] rising edge detect enable
    uint32_t resv6;     // reserved
    uint32_t efen[2];   // [r/w] falling edge detect enable 0
    uint32_t resv7;     // reserved
    uint32_t hen[2];    // [r/w] high detect enable
    uint32_t resv8;     // reserved
    uint32_t len[2];    // [r/w] low detect enable 0
    uint32_t resv9;     // reserved
    uint32_t aren[2];   // [r/w] async rising edge enable 0
    uint32_t resv10;    // reserved
    uint32_t afen[2];   // [r/w] async rising edge enable 0
    uint32_t resv11;    // reserved
    uint32_t pud;       // pull up/down enable
    uint32_t pudclk[2]; // pull up/down enable clock 
} GPIO_Registers;

// Valid GPIO lines are [0,GPIO_MAX)
#define GPIO_MAX 54

// Valid GP function/modes [0,GPF_MAX)
#define GPF_INPUT  0b000
#define GPF_OUTPUT 0b001
#define GPF_ALT0   0b100
#define GPF_ALT1   0b101
#define GPF_ALT2   0b110
#define GPF_ALT3   0b111
#define GPF_ALT4   0b011
#define GPF_ALT5   0b010
#define GPF_MAX    8

#define GPPUD_OFF  0b00
#define GPPUD_DOWN 0b01
#define GPPUD_UP   0b10

// Auxiliary Registers - GPIO has 41 32-bit registers
#define AUX_REGISTERS_OFFSET 0x00215000UL
#define AUX_IRQ         0  // Auxiliary Interrupt status - 3 bits
#define AUX_ENABLES     1  // Auxiliary enables - 3 bits
#define AUX_MU_IO_REG   16 // Mini Uart I/O Data - 8 bits
#define AUX_MU_IER_REG  17 // Mini Uart Interrupt Enable - 8 bits
#define AUX_MU_IIR_REG  18 // Mini Uart Interrupt Identify - 8 bits
#define AUX_MU_LCR_REG  19 // Mini Uart Line Control 8 -bits
#define AUX_MU_MCR_REG  20 // Mini Uart Modem Control - 8 bits
#define AUX_MU_LSR_REG  21 // Mini Uart Line Status - 8 bits
#define AUX_MU_MSR_REG  21 // Mini Uart Modem Status - 8 bits
#define AUX_MU_SCRATCH  23 // Mini Uart Scratch - 8 bits
#define AUX_MU_CNTL_REG 24 // Mini Uart Extra Control - 8 bits
#define AUX_MU_STAT_REG 25 // Mini Uart Extra Status - 32 bits
#define AUX_MU_BAUD_REG 26 // Mini Uart Baudrate - 16 bits

int gpio_set_func(unsigned int gpio, unsigned func);
int gpio_write(unsigned int gpio, unsigned int level);

typedef enum Baud_E {
    BAUD_115200=115200
} Baud;

void uart_init(Baud baud);
void uart_putc(uint8_t ch);
void uart_puts(const char * str);
void uart_putn(int n);
uint8_t uart_getc();

extern volatile IRQ_Registers * irq_registers;
extern volatile System_Timer_Registers * system_timer_registers;
extern volatile Timer_Registers * timer_registers;
extern volatile GPIO_Registers * gpio_registers;
extern volatile uint32_t * aux_registers;

#define SYS_LED_BLUE     22
#define SYS_LED_RED      23
#define SYS_LED_YELLOW   24
#define SYS_LED_GREEN    25


uint64_t system_timer(void);

enum Panic_Code {
    FC_HANG = 1,
    FC_ILLEGAL_ARG,
    FC_ILLEGAL_STATE,
    FC_INVALID_SWI,
    FC_EMPTY_QUEUE,
    FC_NO_PROCESS,
    FC_WRONG_PROCESS,
    FC_OUT_OF_PROC,
    FC_INVALID_PROC_STATE,
    FC_INVALID_PROC_MAGIC,
    FC_STACK_OVERFLOW,
    FC_OUT_OF_MON,
    FC_INVALID_MON_STATE,
    FC_ALREADY_INITIALIZED,
};

void panic(int code);

uint32_t busy_wait_millis(uint32_t millis);

#endif // __BCM2835_H__
