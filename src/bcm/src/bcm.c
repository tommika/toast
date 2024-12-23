// Copyright (c) 2020,2024 Thomas Mikalsen. Subject to the MIT License 
#include <stdint.h>

// See: BCM2835 ARM Peripherals
//  https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf
// 
// For GPIO related information, see section 6.
//

#ifndef __bare_metal__
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>

#endif

#include "bcm.h"
#include "util.h"

static volatile GPIO_Registers * gpio_registers = NULL;
static volatile PWM_Registers * pwm_registers = NULL;
static volatile CM_Registers * cm_registers = NULL;
static volatile Aux_Registers * aux_registers = NULL;

static uint32_t system_clock_hz = 0;

// TODO - add param to control which HW features to enable: e.g. gpio, pwm, uart, ...

int bcm_init(RPi_Model model, unsigned core_freq_mhz) {
    // First, determine the base offset to the BCM* peripheral registers, based on the particular
    // RPi model.
    uint32_t bcm_base = 0;
    switch(model) {
        default:
            fprintf(stderr,"Unrecognized model: model=%d\n",model);
            return -1;
        case RPi_1:
            bcm_base = BCM_BASE_RPI1;
            break;
        case RPi_2:
            bcm_base = BCM_BASE_RPI2;
            break;
        case RPi_3:
            bcm_base = BCM_BASE_RPI3;
            break;
    }
    fprintf(stderr,"model=%u, bcm_base=%x, core_freq_mhz=%d\n",model,bcm_base,core_freq_mhz);
    system_clock_hz = core_freq_mhz * 1000000;
#ifdef __bare_metal__
    // Running on bare-metal: index directly into physical memory
    gpio_registers = (GPIO_Registers*)(bcm_base + GPIO_REGISTERS_OFFSET);
#else
    // Running on Linux: memory-map the BCM* peripheral registers into our address space.
    // To do this, we'll first open the `/dev/mem` device, which provides direct access to 
    // the physical memory.
    int fd;
    if ((fd = open("/dev/mem", O_RDWR | O_SYNC) ) < 0) {
        fprintf(stderr,"Failed to open /dev/mem: %s\n", strerror(errno));
        return -1;
    }

    // Memory-map the GPIO registers
    gpio_registers = (GPIO_Registers*)mmap(0, 
                                           sizeof(GPIO_Registers), 
                                           PROT_READ|PROT_WRITE, 
                                           MAP_SHARED, 
                                           fd, 
                                           bcm_base + GPIO_REGISTERS_OFFSET);
    if(gpio_registers==MAP_FAILED) {
        fprintf(stderr,"Failed memory-map GPIO registers: %s\n", strerror(errno));
        return -2;
    }

    // Memory-map the Clock Manager registers
    cm_registers = (CM_Registers*)mmap(0, 
                                           512, 
                                           PROT_READ|PROT_WRITE, 
                                           MAP_SHARED, 
                                           fd, 
                                           bcm_base + CM_REGISTERS_OFFSET);
    if(cm_registers==MAP_FAILED) {
        fprintf(stderr,"Failed memory-map Clock Manager registers: %s\n", strerror(errno));
        return -3;
    }

    // Memory-map the PWM registers
    pwm_registers = (PWM_Registers*)mmap(0, 
                                           sizeof(PWM_Registers), 
                                           PROT_READ|PROT_WRITE, 
                                           MAP_SHARED, 
                                           fd, 
                                           bcm_base + PWM_REGISTERS_OFFSET);
    if(gpio_registers==MAP_FAILED) {
        fprintf(stderr,"Failed memory-map PWM registers: %s\n", strerror(errno));
        return -2;
    }

    // Memory-map the aux registers
    assert(sizeof(Aux_Registers)==AUX_REGISTERS_SIZE);
    aux_registers = (Aux_Registers*)mmap(0, 
                                           sizeof(Aux_Registers), 
                                           PROT_READ|PROT_WRITE, 
                                           MAP_SHARED, 
                                           fd, 
                                           bcm_base + AUX_REGISTERS_OFFSET);
    if(aux_registers==MAP_FAILED) {
        fprintf(stderr,"Failed memory-map AUX registers: %s\n", strerror(errno));
        return -2;
    }

    // Now that we've successfully memory-mapped the peripheral registers, 
    // we can close the `/dev/mem` device.
    close(fd);
#endif
    return 0;   
}

/** Set the function function (INPUT, OUTPUT, ALTn) for the given GPIO pin.
 * 
 *  The GPIO function is represented using a 3-bit value. The function selected 
 *  for a given pin is stored in one of the 5 function select registers (GPFSEL0 - 5).
 *  To select the function for  a given pin, we must first determine the
 *  register for that given pin, and then the bits within that register.
 *  Each of the 32-bit registers can stores function values for up to 10 pins
 *  (3 bits * 10 pins = 30bits <= 32)
 */
int gpio_set_func(unsigned int gpio, unsigned char func) {
    if(gpio>=GPIO_MAX) {
        return -1;
    }
    if(func>=GPF_MAX) {
        return -2;
    }
    int fsel = gpio / 10;       // determine the register
    int shift = (gpio%10) * 3;  // determine the starting bit position
    volatile uint32_t * reg = (&gpio_registers->fsel[fsel]);
    *reg = (*reg & ~(7<<shift)) | (func<<shift);
    return 0;
}

/** Enable/disable internal pull-up/down resistor for the given GPIO pin */
int gpio_set_pud(unsigned int gpio, unsigned char pud) {
    if(gpio>=GPIO_MAX) {
        return -1;
    }
    if(pud>=GPPUD_MAX) {
        return -2;
    }
    // Determine bank & bit for given gpio pin
    uint32_t bank = (gpio>>5) ? 1 : 0; // determine which bank (0 or 1)
    uint32_t bit = (1<<(gpio&0x1F));   // determine bit within bank 
    // Select pudclk register for the determined bank
    volatile uint32_t * pudclk_reg = (&gpio_registers->pudclk[bank]);

    // 1. Write to GPPUD to set the required control signal (i.e. Pull-up or Pull-Down or neither to remove the current Pull-up/down) 
    gpio_registers->pud = pud;
    // 2. Wait 150 cycles – this provides the required set-up time for the control signal 
    busywait(150);
    // 3. Write to GPPUDCLK0/1 to clock the control signal into the GPIO pads you wish to modify  
    //    – NOTE only the pads which receive a clock will be modified, all others will retain their previous state.
    *pudclk_reg = bit;
    // 4. Wait 150 cycles – this provides the required hold time for the control signal 
    busywait(150);
    // 5. Write to GPPUD to remove the control signal 
    gpio_registers->pud = 0;
    // 6. Write to GPPUDCLK0/1 to remove the clock
    *pudclk_reg = 0;

    return 0;
}

int gpio_write(unsigned int gpio, unsigned int level) {
    if(gpio>=GPIO_MAX) {
        return -1;
    }
    uint32_t bank = (gpio>>5) ? 1 : 0; // determine which bank (0 or 1)
    uint32_t bit = (1<<(gpio&0x1F));   // determine bit within bank 
    volatile uint32_t * reg;
    if(level) {
        // Set GPIO
        reg = &gpio_registers->set[bank];
    } else {
        // Clear GPIO
        reg = &gpio_registers->clr[bank];
    }
    *reg = bit;
    return 0;
}

int gpio_read(unsigned int gpio) {
    if(gpio>=GPIO_MAX) {
        return -1;
    }
    uint32_t bank = (gpio>>5) ? 1 : 0; // determine which bank (0 or 1)
    uint32_t bit = (1<<(gpio&0x1F));   // determine bit within bank 
    volatile uint32_t * reg = &gpio_registers->lev[bank];
    return (*reg & bit) != 0;
}

int pwm_enable(unsigned range) {
    volatile unsigned int n;

    fprintf(stderr,"Starting pwm\n");
    volatile uint32_t * cm_ctl = cm_registers + (CM_PWM_OFFSET/4);
    volatile uint32_t * cm_div = cm_ctl + 1;

    // Stop PWM
    fprintf(stderr,"disabling pwm\n");
    pwm_registers->ctl = 0;
    pwm_registers->dat1 = 0;
    busywait(150);

    fprintf(stderr,"disabling clock\n");
    // Stop the clock, to avoid any "glitches" (technical term)
    *cm_ctl = CM_PASSWORD | (*cm_ctl & ~CM_CTL_ENAB);
    // Wait for clock to stop
    fprintf(stderr,"waiting for clock to stop...");
    busywait(150);
    while(*cm_ctl & CM_CTL_BUSY) {
        n++;
    }
    fprintf(stderr,"clock has stopped\n");
    // Start clock using given divisor
    *cm_div = CM_PASSWORD | (0xFFF<<12);
    *cm_ctl = CM_PASSWORD | CM_CTL_ENAB | CM_CTL_SRC_OSCILLATOR;
    fprintf(stderr,"waiting for clock to start ...");
    busywait(150);
    while(!(*cm_ctl & CM_CTL_BUSY)) {
        n++;
    }
    fprintf(stderr,"clock is running\n");

    // Enable PWM
    fprintf(stderr,"enabling pwm\n");
    pwm_registers->rng1 = range;
    pwm_registers->dat1 = 0;
    pwm_registers->ctl = 1;

    return 0;
}

void pwm_disable(unsigned range, unsigned val) {
    volatile unsigned int n;

    fprintf(stderr,"disabling pwm\n");
    pwm_registers->dat1 = 0;
    pwm_registers->ctl = 0;

    fprintf(stderr,"disabling clock\n");
    volatile uint32_t * cm_ctl = cm_registers + (CM_PWM_OFFSET/4);
    *cm_ctl = CM_PASSWORD | (*cm_ctl & ~CM_CTL_ENAB);
        fprintf(stderr,"waiting for clock to stop...");
    while(*cm_ctl & CM_CTL_BUSY) {
        n++;
    }
    fprintf(stderr,"clock has stopped\n");
}

void pwm_set(unsigned val) {
    pwm_registers->dat1 = val;
}


// See BCM2835 section 2.2 - Mini UART
//
// To compute baudrate value, we need to know the system clock frequency
//
//    baud = (system_clock_freq) / (8 * ( baudrate_reg + 1 ) )
//    baud * 8 * ( baudrate_reg + 1 ) = system_clock_freq
//    baudrate_reg + 1 = system_clock_freq / (baud * 8)
//    baudrate_reg = (system_clock_freq / (baud * 8)) - 1
//
// E.g.  
//   baud = 115200
//   system_clock_freq = 250000000 (250 MHz)
//   buadrate_reg = (250000000 / (115200 * 8)) - 1
//   buadrate_reg = (250000000 / 921600) - 1
//   buadrate_reg = 270
//
void uart_init(uint32_t baud) {
    uint32_t baudrate = (system_clock_hz / (baud*8) ) - 1;
    fprintf(stderr,"baud=%u, baudrate=%u\n",baud,baudrate);
    gpio_set_func(14,GPF_ALT5);
    gpio_set_func(15,GPF_ALT5);
    gpio_set_pud(14,GPPUD_OFF);
    gpio_set_pud(15,GPPUD_OFF);

    aux_registers->enables = 1;   // enable the mini-uart 
    aux_registers->mu_cntl = 0;   // disable tx/rx 
    aux_registers->mu_ier = 0;    // disable tx/rx interrupts (for now -- need to revisit for interupt driven I/O)
    aux_registers->mu_lcr = 3;    // 8-bit mode
    aux_registers->mu_mcr = 0;
    aux_registers->mu_baud = baudrate;
    aux_registers->mu_cntl = 3;   // enable tx/rx 
}

static volatile int _uart_cancel_io = 0;
void uart_cancel() {
    _uart_cancel_io = 1;
}

void uart_putc(uint8_t c) {
    // wait until device is ready to accept data
    while(1) {
        volatile uint32_t val = aux_registers->mu_lsr;
        if(val & 0x20) {
            break;
        }
    }
    aux_registers->mu_io = c;
}

void uart_puts(const char * str) {
    while(*str) {
        uart_putc(*str++);
    }
}

#ifndef __bare_metal__
uint8_t uart_getc() {
    while(!_uart_cancel_io) {
        volatile uint32_t val = aux_registers->mu_lsr;
        if(val & 0x01) {
            return aux_registers->mu_io & 0xFF;
        }
    }
    return 0;
}

/** Read at most one line of characters, from the uart device, into the given 
 *  fixed-size buffer.
 * 
 *  If the newline character ('\n') is encountered before the given buffer is 
 *  full, then the newline is replaced with a null-terminator and the 
 *  string-length of the buffer is returned; the returned string length
 *  does _not_ count the null-terminator and is necessarily less-than the 
 *  buffer size in this case.
 *
 *  If the buffer is filled before the newline character ('\n') is encountered,
 *  then the return value is the given buffer size, indicating that there 
 *  is more data to be read for the current line.
 * 
 *  If I/O is canceled while waiting for input, this function will return
 *  as if the newline character was encountered.
 * 
 * Optional flags:
 * - IGNORE_CR - carriage return ('\r') characters are ignored.
 * 
 * E.g.,
 * ---
 *   // Copy one line of input from uart to stdout
 *   char buffer[128];
 *   size_t len;
 *   while((len=uart_getline(buffer,sizeof(buffer),IGNORE_CR))==sizeof(buffer)) {
 *       fwrite(buffer,1,len,stdout);
 *   }
 *   fwrite(buffer,1,len,stdout);
 * ---
 */
size_t
uart_getline(char *buff, size_t buff_size, uint16_t flags)
{
    char *pch = buff;  // current pointer into the buffer
    size_t cch = 0;    // number of characters read into the buffer so far
    // if data is not immediately available, poll at at least ~1ns interval
    while(!_uart_cancel_io && !(aux_registers->mu_lsr & 0x01)) {
        sleep_nanos(1);
    }
    if(_uart_cancel_io) {
        *pch = 0;
        return 0;
    }
    while(cch<buff_size) {
        while(!_uart_cancel_io && !(aux_registers->mu_lsr & 0x01)) {
            // no data available; spin
        }
        char ch = aux_registers->mu_io & 0xFF;
        if(_uart_cancel_io || ch=='\n') {
            // encountered a newline, or we've been canceled (at the hotel?)
            // add null-terminator and return
            *pch++ = 0;
            break;
        }
        if(ch=='\r' && (flags & IGNORE_CR)) {
            continue;
        }
        // add character to the buffer
        *pch++ = ch;
        cch++;
    }
    return cch;
}
#endif // __bare_metal__