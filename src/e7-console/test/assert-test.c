#include "assert.h"
#include <stdio.h>

int main(int argc, char ** argv) {
    ASSERT(1==1,0);
	printf("expect this to panic\n");
    ASSERT(1==0,0);
    return 0;
}
