// Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License
#include "toast.h"

volatile uint32_t ready = false;
uint32_t green_blinker(uint32_t);
uint32_t yellow_blinker(uint32_t);

uint32_t app_main(uint32_t init_param) {
    sys_log("app_main is running");
    ready = false;
    sys_set_led(SYS_LED_RED,1);

    // Create a monitor
    uint32_t mid = sys_mon_create();

    // Fork-off a couple processes,passing in the monitor
    sys_log("app_main is forking child processes");
    sys_fork(green_blinker,mid,0);
    sys_fork(yellow_blinker,mid,0);

    sys_log("app_main is sleeping for a bit");
    sys_sleep_millis(2000);

    // Notify child processes that we're ready
    sys_log("app_main is notifying child processes");
    sys_mon_enter(mid);
    ready = true;
    sys_mon_notify(mid);
    sys_mon_exit(mid);
    // Exit this process (child processes will continue running)
    sys_set_led(SYS_LED_RED,0);
    sys_log("app_main is exiting");
    return 0;
}

uint32_t green_blinker(uint32_t mid) {
    // Don't start until ready
    sys_log("green_blinker is waiting for ready signal...");
    sys_mon_enter(mid);
    while(!ready) {
        sys_mon_wait(mid);
    }
    // notify anyone else that might be waiting
    sys_mon_notify(mid);
    sys_mon_exit(mid);
    sys_log("green_blinker is running");
    while(1) {
        // ON
        sys_set_led(SYS_LED_GREEN,1);
        sys_sleep_millis(800);
        // OFF
        sys_set_led(SYS_LED_GREEN,0);
        sys_sleep_millis(200);
    }
}

uint32_t yellow_blinker(uint32_t mid) {
    // Don't start until ready
    sys_log("yellow_blinker is waiting for ready signal...");
    sys_mon_enter(mid);
    while(!ready) {
        sys_mon_wait(mid);
    }
    // notify anyone else that might be waiting
    sys_mon_notify(mid);
    sys_mon_exit(mid);
    sys_log("yellow_blinker is running");
    while(1) {
        // ON
        sys_set_led(SYS_LED_YELLOW,1);
        sys_sleep_millis(300);
        // OFF
        sys_set_led(SYS_LED_YELLOW,0);
        sys_sleep_millis(300);
    }
}
