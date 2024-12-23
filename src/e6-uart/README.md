<!--
Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License
-->
UART
====

Experimenting with the UART interface

See project [README.md](../../README.md) for pre-reqs and such.

Try it
------
Insert SD card into your development workstation, and run:
```
make install eject
```

Then insert the SD card it into your RPi 1 and start it up.

Changes
-------
* Refactored gpio.c/h into bcm2835.c/h
* Added uart commands: `uart_init`, `uart_putc`, `uart_puts`, `uart-getc`
* Added `sys_println` system call
* Update app to call `sys_println`

TODO
----
Would like the uart IO to be interrupt driven.

Notes & References
------------------

### Technical References:
* [ARM Processor Exceptions](https://developer.arm.com/documentation/dui0040/d/handling-processor-exceptions)
* [ARM Interrupt Handlers](https://developer.arm.com/documentation/dui0040/d/handling-processor-exceptions/interrupt-handlers)
* [ARM CPS - Change Processor State](https://developer.arm.com/documentation/dui0473/k/arm-and-thumb-instructions/cps)
* [BCM2835 ARM Peripherals Guide for Raspberry Pi](https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf)
* https://developer.arm.com/documentation/dui0056/d/handling-processor-exceptions/interrupt-handlers/example-interrupt-handlers-in-assembly-language
* https://developer.arm.com/documentation/dui0471/m/handling-processor-exceptions/context-switch


* https://www.keil.com/support/man/docs/armasm/armasm_dom1359731127340.htm
