// Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License
#include <stdbool.h>
#include "str.h"

char * reverse(char * pch, int len)  { 
    for(int ich_s=0, ich_e=len-1; ich_s<ich_e; ich_s++, ich_e--) {
        char ch_t = pch[ich_s];
        pch[ich_s] = pch[ich_e];
        pch[ich_e] = ch_t;
    }
    return pch;
}

char * itoa(int val, char* buff, int base)  { 
    char * pch = buff;
    if(val == 0) { 
        *pch++ = '0'; 
        *pch = 0;
        return buff; 
    }
    bool neg = (val<0);
    if(neg) {
        val = -val; 
    } 
    while(val) { 
        int r = val % base; 
        *pch++ = (r > 9) ? (r-10) + 'A' : r + '0'; 
        val /= base; 
    } 
    if(neg) { 
        *pch = '-';
    }
    *pch = 0;
    return reverse(buff,pch-buff); 
} 
