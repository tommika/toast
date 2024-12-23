<!--
Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License 
-->
Context Switch
==============

Experimenting with preemptive multitasking and context switching between User-mode processes.

See project [README.md](../../README.md) for pre-reqs and such.

Try it
------
Insert SD card into your development workstation, and run:
```
make install eject
```

Then insert the SD card it into your RPi 1 and start it up.

Context Switching
-----------------
The "Supervisor" component is extended as follows:

* Defines a process control structure -- ` Process` -- used to store registers, stack memory and other process bookkeeping.
* `process_ready` - Priority-queue of processes that are ready for execution
* Function for creating new processes
* Process Scheduler - Inserts running process into the `process_ready` queue, and de-queues next process
* Interrupt-handler for context-switching between current process and process returned from the scheduler
* Supervisor initialization that creates processes and schedules initial process.
* SWI (software interrupt) handler modified to allow a system call to context-switch (e.g., an explicit 'yield', 'wait', or some blocking call)
* Pointer to the current running process is maintained in the IRQ-Mode stackpointer (sp_IRQ)

Notes & References
------------------

### Software Interrupt Handler (SWI)
https://developer.arm.com/documentation/dui0040/d/handling-processor-exceptions/swi-handlers/using-swis-in-supervisor-mode?lang=en

> When a SWI instruction is executed, the processor enters supervisor mode, 
> the CPSR is stored into spsr_SVC, and the return address is stored in lr_SVC"

### Technical References:
* [ARM Processor Exceptions](https://developer.arm.com/documentation/dui0040/d/handling-processor-exceptions)
* [ARM Interrupt Handlers](https://developer.arm.com/documentation/dui0040/d/handling-processor-exceptions/interrupt-handlers)
* [ARM CPS - Change Processor State](https://developer.arm.com/documentation/dui0473/k/arm-and-thumb-instructions/cps)
* [BCM2835 ARM Peripherals Guide for Raspberry Pi](https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf)
* https://developer.arm.com/documentation/dui0056/d/handling-processor-exceptions/interrupt-handlers/example-interrupt-handlers-in-assembly-language
* https://developer.arm.com/documentation/dui0471/m/handling-processor-exceptions/context-switch

