// Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License
#include "util.h"

NOINLINE void busywait(unsigned int n) {
    volatile unsigned count = 0;
    for(unsigned i=0; i<n; i++) {
        count++;
    }
}
