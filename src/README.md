<!--
Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License 
-->
There's a progression to these experiments:


Completed
---------
* **e1-blink**: Simple bare-metal LED blinker, using ARM assembly and c code
* **e2-arm-modes**: Experiment with changing ARM modes and SWI handlers
* **e3-timer**: Experiment with the BCM2835 timer peripheral and IRQs
* **e4-context-switch**: Things start to get interesting here, as we combine e2 & e3 to provide a basic preemptive multi-tasking Supervisor
* **e5-monitor-sync**: Adds support for "monitor" concurrency/synchronization primitives
* **e6-uart**: Adds support for UART interface and basic i/o to dumb terminal
* **e7-console**: Adds an interactive console for the supervisor

In Progress
-----------

TODO
----
* Basic I/O - beyond LEDs
* Run on RaspberryPi 3 
