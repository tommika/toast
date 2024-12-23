// Copyright (c) 2020,2024 Thomas Mikalsen. Subject to the MIT License 
#ifndef __BCM_H__
#define __BCM_H__

#include <stdint.h>
#ifndef NULL
#define NULL ((void*)0)
#endif

typedef enum {
    RPi_1 = 1,
    RPi_2 = 2,
    RPi_3 = 3
} RPi_Model;

int bcm_init(RPi_Model model, uint32_t core_freq_mhz);

// See: BCM2835 ARM Peripherals
//  https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf

// 1.2.3 ARM physical addresses
// Physical addresses start at 0x00000000 for RAM.
//
// The bus addresses for peripherals are set up to map onto the peripheral 
// bus address range starting at 0x7E000000.

#define BCM_BASE_RPI1 0x20000000UL
#define BCM_BASE_RPI2 0x3F000000UL
#define BCM_BASE_RPI3 0x3F000000UL

// Valid GPIO lines are [0,GPIO_MAX)
#define GPIO_MAX 54

// GPIO Registers - GPIO has 41 32-bit registers
#define GPIO_REGISTERS_OFFSET 0x00200000UL
volatile typedef struct {
    uint32_t fsel[6];   // [r/w] function select 0-5
    uint32_t resv1;     // reserved
    uint32_t set[2];    // [w] set output
    uint32_t resv2;     // reserved
    uint32_t clr[2];    // [w] clear output
    uint32_t resv3;     // reserved
    uint32_t lev[2];    // [w] level
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

// Pull up/down
#define GPPUD_OFF  0b00
#define GPPUD_DOWN 0b01
#define GPPUD_UP   0b10
#define GPPUD_MAX  0b11

// Clock Manager (CM) Registers
// - this offset is not documented properly in the BCM manual
// See
//   https://elinux.org/BCM2835_registers#CM
#define CM_REGISTERS_OFFSET  0x00101000UL
volatile typedef uint32_t CM_Registers;
#define CM_REGISTERS_SIZE    512
#define CM_GP0_OFFSET 0x070
#define CM_PWM_OFFSET 0x0A0

#define CM_CTL_ENAB  (1<<4)
#define CM_CTL_KILL  (1<<5)
#define CM_CTL_BUSY  (1<<7)
#define CM_PASSWORD  (0x5a<<24)

// Clock sources
#define CM_CTL_SRC_OSCILLATOR 1
#define OSCILLATOR_FREQ 19200000 // 1.92 MHz

// PWM Registers - PWM has 8 32-bit registers
// - https://elinux.org/BCM2835_datasheet_errata#p141
#define PWM_REGISTERS_OFFSET 0x0020C000UL
volatile typedef struct {
    uint32_t ctl;  // Control
    uint32_t sta;  // Status
    uint32_t dmac; // DMA Configuration
    uint32_t rscv; 
    uint32_t rng1; // Channel 1 Range
    uint32_t dat1; // Channel 1 Data
    uint32_t fif1; // FIFO Input
    uint32_t rng2; // Channel 2 Range
    uint32_t dat2; // Channel 2 Data
} PWM_Registers;

int gpio_set_func(unsigned int gpio, unsigned char func);
int gpio_set_pud(unsigned int gpio, unsigned char pud);
int gpio_write(unsigned int gpio, unsigned level);
int gpio_read(unsigned int gpio);
int pwm_enable(unsigned range);
void pwm_disable();
void pwm_set(unsigned val);

// See BCM2835 - Table 6-28 synopsis
#define AUX_REGISTERS_OFFSET 0x00215000UL
#define AUX_REGISTERS_COUNT 54
#define AUX_REGISTERS_SIZE (AUX_REGISTERS_COUNT<<2)

// 54 registers
volatile typedef struct {
    uint32_t irq;        // 0      : Auxiliary Interrupt status
    uint32_t enables;    // 1      : Auxiliary enables
    uint32_t _[14];      // 2-15   : Unused
    uint32_t mu_io;      // 16     : Mini Uart I/O Data
    uint32_t mu_ier;     // 17     : Mini Uart Interrupt Enable
    uint32_t mu_iir;     // 18     : Mini Uart Interrupt Identify
    uint32_t mu_lcr;     // 19     : Mini Uart Line Control
    uint32_t mu_mcr;     // 20     : Mini Uart Modem Control
    uint32_t mu_lsr;     // 21     : Mini Uart Line Status
    uint32_t mu_msr;     // 22     : Mini Uart Modem Status
    uint32_t mu_scratch; // 23     : Mini Uart Scratch
    uint32_t mu_cntl;    // 24     : Mini Uart Extra Control
    uint32_t mu_stat;    // 25     : Mini Uart Extra Status
    uint32_t mu_baud;    // 26     : Mini Uart Baudrate
    uint32_t __[5];      // 27-31  : Unused
    uint32_t spi[22];    // 32...  : SPI registers... don't care right now
} Aux_Registers;

#define BAUD_115200 115200
#define BAUD_9600   9600
#define IGNORE_CR 0x0001
void uart_init(uint32_t baud);
void uart_putc(uint8_t ch);
void uart_puts(const char * str);
void uart_putn(int n);
uint8_t uart_getc();
size_t uart_getline(char * buff, size_t buff_size, uint16_t flags);
void uart_cancel();

static inline void sleep_nanos(unsigned nanos) {
    struct timespec ts = {.tv_sec = 0,.tv_nsec = nanos};
    nanosleep(&ts,NULL);
}

static inline void sleep_millis(unsigned millis) {
    struct timespec ts = {.tv_sec = millis / 1000,.tv_nsec = (millis%1000) * 1000 * 1000};
    nanosleep(&ts,NULL);
}

static inline void busywait(volatile unsigned int n) {
    while(n-->0);
}

#endif // __BCM_H__