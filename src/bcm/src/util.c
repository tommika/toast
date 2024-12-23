// Copyright (c) 2020,2024 Thomas Mikalsen. Subject to the MIT License 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <error.h>
#include <stdlib.h>

/** Drop superuser privileges.
 * 
 *  If running under sudo, this is done by changing to the user/group that
 *  was active when sudo was called.
 * 
*/
int drop_root(void) {
	if (getuid() != 0) {
        // Not running as root; all set
		return 0;
	}
    uid_t uid; // UID to switch to
    gid_t gid; // GID to switch to

    // The SUDO_UID & SUDO_GID env vars are set when the program
    // was run via sudo.
    const char * sudo_uid =secure_getenv("SUDO_UID");
    const char * sudo_gid = secure_getenv("SUDO_GID");
    if(!sudo_uid || !sudo_gid) {
        // Not running under sudo
        fprintf(stderr,"Not running under sudo; can't determine non-root user\n");
        return -1;
    }

    // Use SUDO_UID and SUDO_GID
    uid = strtoll(sudo_uid,NULL,10);
    if(uid==0) {
        perror("Failed to parse UID");
        return -1;
    } 
    gid = strtoll(sudo_gid,NULL,10);
    if(gid==0) {
        perror("Failed to parse GID");
        return -1;
    }

    // The order here is important:
    //   first set group, 
    //   then set the user
    if (setgid(gid)) {
        perror("setgid failed");
        return -1;    
    }
    if(setuid(uid)) {
        perror("setuid failed");
        return -1;
    }
    if(getuid()==0) {
        fprintf(stderr,"Failed to change user\n");
        return -1;
    }
    return 0;
}