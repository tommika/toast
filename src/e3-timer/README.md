<!--
Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License 
-->
Timer
=====

Experimenting with Interrupts and the BCM2835 Timer

See project [README.md](../../README.md) for pre-reqs and such.

Try it
------
Insert SD card into your development workstation, and run:
```
make install eject
```

Then insert the SD card it into your RPi 1 and start it up.

In this code, interrupts are enabled when the application runs in User mode:
```
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
```

If we wanted to enable interrupts for Supervisor mode, we would use something like so:
```
.global enable_interrupts
enable_interrupts:
    mrs     r0, cpsr
    bic     r0, r0, #0x80
    msr     cpsr_c, r0
    mov     pc, lr

```

Notes & References
------------------

Technical References:
* [ARM Processor Exceptions](https://developer.arm.com/documentation/dui0040/d/handling-processor-exceptions)
* [ARM Interrupt Handlers](https://developer.arm.com/documentation/dui0040/d/handling-processor-exceptions/interrupt-handlers)
* [ARM CPS - Change Processor State](https://developer.arm.com/documentation/dui0473/k/arm-and-thumb-instructions/cps)
* [BCM2835 ARM Peripherals Guide for Raspberry Pi](https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf)


Other stuff I found on the Web that was helpful:
* https://www.valvers.com/open-software/raspberry-pi/bare-metal-programming-in-c-part-4/
