<!--
Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License 
-->
Monitors and Process Synchronization
====================================

Experimenting with the "monitor" primitives and process synchronization.

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
* Added `sys_sleep_millis` system call and sleep queue
* Replaced app process calls to "busywait" with calls to `sys_sleep_millis`
* Get current time from BCM 2835 System timer
* Moved some constants into c-style header file for sharing between assembly and c code. Had to rename `start.s` to `start.S` (capital `S`) to get gcc to run pre-processor.
* Added `sys_fork` system call; main application now forks child processes
* Added Monitor system calls: `sys_mon_create`, `sys_mon_enter`, `sys_mon_exit`, `sys_mon_wait`, `sys_mon_notify`

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