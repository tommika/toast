// Copyright (c) 2020,2024 Thomas Mikalsen. Subject to the MIT License 
//
// Read data stream from GPS receiver via UART
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
    unsigned int uart_baud;        //  
} config;

int main(int argc, char ** argv) {
    // Default config
    //  TODO: allow to be overriden from command line or config file
    config config = {
        .rpi_model = RPi_3,
        .core_freq_mhz = 250, // Must match core_freq in /boot/config.txt
        .uart_baud = BAUD_115200,
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

    uart_init(config.uart_baud);

    fprintf(stderr,"Reading data from GPSr; use Ctrl-C to quit\n");
    while(!shutdown) {
        char buffer[128];
        size_t len;
        while((len=uart_getline(buffer,sizeof(buffer),IGNORE_CR))==sizeof(buffer)) {
            fwrite(buffer,1,len,stdout);
        }
        fwrite(buffer,1,len,stdout);
        fprintf(stdout,"\n");
    }
    fprintf(stderr,"\nShutting down...\n");

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
        uart_cancel();
		break;
	}
}