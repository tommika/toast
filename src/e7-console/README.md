<!--
Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License 
-->
Console
=======

Adding an interactive console for the supervisor


See project [README.md](../../README.md) for pre-reqs and such.

Try it
------
Insert SD card into your development workstation, and run:
```
make install eject
```

Then insert the SD card it into your RPi 1 and start it up.

Monitor TTY/
```
screen /dev/ttyUSB0 115200
```
To exit from `screen`, `Ctrl+a, \`

To disconnect from `screen`, `Ctrl+a, Ctrl+d`,
and to resume `screen -r`.


Changes
-------
Add test cases that can build and run on the host dev machine

TODO
----
Would like the uart IO to be interrupt driven.
Console application - accepts commands from the console.


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
