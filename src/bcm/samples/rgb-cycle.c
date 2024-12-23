// Copyright (c) 2020,2024 Thomas Mikalsen. Subject to the MIT License 
//
// Cycle through LEDs
//
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <string.h>

#include <bcm.h>
#include <util.h>

static void sigint_handler(int sig);

static volatile int shutdown = 0;  // Exit when non-zero

typedef struct {
    RPi_Model rpi_model;           // Specific model
    unsigned int core_freq_mhz;    // Must match core_freq in /boot/config.txt
    unsigned int gpio_i_cycle;     // Input pin for color cycle button/switch
    unsigned int gpio_i_cycle_pud; //   pull up/down mode for this pin

    unsigned int gpio_o_red;       // Output pin for red led
    unsigned int gpio_o_green;     // Output pin for green led
    unsigned int gpio_o_blue;      // Output pin for blue led

    unsigned int gpio_pwm;         // Output pin for PWM led
    unsigned int gpio_pwm_func;    // ALT func for PWM pin (rg, ALT0)
    unsigned int delay_millis;     // Delay in millis
} config;

int main(int argc, char ** argv) {

    // Default config
    //  TODO: allow to be overriden from command line or config file
    config config = {
        .rpi_model = RPi_3,
        .core_freq_mhz = 250,  // must match core_freq in /boot/config.txt
        .gpio_i_cycle = 16,
        .gpio_i_cycle_pud = GPPUD_UP,
        .gpio_o_red = 23,
        .gpio_o_green = 24,
        .gpio_o_blue = 25,
        .gpio_pwm = 12,
        .gpio_pwm_func = GPF_ALT0,
        .delay_millis = 200,
    };

    if(getuid()!=0) {
        fprintf(stderr,"This program requires superuser privileges.\n");
        fprintf(stderr,"Please run as root, for example, using\n");
        fprintf(stderr,"  sudo %s\n",argv[0]);
        exit(1);
    }
    fprintf(stderr,"Initializing BCM...\n");
    if(bcm_init(config.rpi_model,config.core_freq_mhz)!=0) {
        fprintf(stderr,"Initialization failed\n");
        exit(2);
    }
    
    fprintf(stderr,"Dropping superuser privileges...\n");
    if(drop_root()!=0) {
        fprintf(stderr,"Failed to drop root privileges \n");
        exit(2);
    }
    if(getuid()==0) {
        fprintf(stderr,"Expected to be non-root at this point\n");
        exit(2);
    }

    fprintf(stderr,"Initialization complete\n");

    signal(SIGINT, sigint_handler);
	signal(SIGTERM, sigint_handler);

    // Set-up GPIO based on config
    gpio_set_func(config.gpio_pwm,config.gpio_pwm_func);
    gpio_set_func(config.gpio_i_cycle,GPF_INPUT);
    gpio_set_pud(config.gpio_i_cycle,config.gpio_i_cycle_pud);
    gpio_set_func(config.gpio_o_red,GPF_OUTPUT);
    gpio_set_func(config.gpio_o_green,GPF_OUTPUT);
    gpio_set_func(config.gpio_o_blue,GPF_OUTPUT);

    unsigned pwm_range = 16;
    unsigned pwm_val = 0;
    pwm_enable(pwm_range);

    unsigned char rgb = 0b000; // initial state is all off (3-bits per pixel :-) )

    fprintf(stderr,"Cycling RGB LEDs; use Ctrl-C to quit\n");
    int pwm_inc = 1;
    while(!shutdown) {
        pwm_set(pwm_val);
        pwm_val += pwm_inc;
        if(pwm_val>=(pwm_range-1)) {
            pwm_inc = -1;
        } else if (pwm_val==0) {
            pwm_inc = 1;
        }
        gpio_write(config.gpio_o_red,rgb&1);
        gpio_write(config.gpio_o_green,rgb&2);
        gpio_write(config.gpio_o_blue,rgb&4);
        sleep_millis(100);
        int button = gpio_read(config.gpio_i_cycle);
        if(button) {
            // cycle through the 3 bits per pixel :-)
            rgb = (rgb+1) % 8;
        }
    }
    fprintf(stderr,"\nShutting down...\n");

    pwm_disable();
    gpio_write(config.gpio_pwm,0);
    gpio_set_func(config.gpio_pwm,GPF_INPUT);

    // Turn off LEDs
    gpio_write(config.gpio_o_red,0);
    gpio_write(config.gpio_o_green,0);
    gpio_write(config.gpio_o_blue,0);

    fprintf(stderr,"Exiting\n");
}

static void sigint_handler(int sig) {
	switch(sig) {
	default:
		fprintf(stderr,"Unexpected signal: %d\n",sig);
		break;
	case SIGINT:
	case SIGTERM:
		fprintf(stderr,"\nReceived shutdown signal\n");
    	shutdown += 1;
        if(shutdown>2) {
            // Seems the application is not responding,
            // so force it to exit
            exit(1);
        }
		break;
	}
}