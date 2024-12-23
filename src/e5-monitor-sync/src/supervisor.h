// Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License
#ifndef __SUPER_H__
#define __SUPER_H__

enum Panic_Code {
    FC_DEBUG = 1,
    FC_ILLEGAL_ARG,
    FC_ILLEGAL_STATE,
    FC_INVALID_SWI,
    FC_EMPTY_QUEUE,
    FC_NO_PROCESS,
    FC_WRONG_PROCESS,
    FC_OUT_OF_PROC,
    FC_INVALID_PROC_STATE,
    FC_INVALID_PROC_MAGIC,
    FC_STACK_OVERFLOW,
    FC_OUT_OF_MON,
    FC_INVALID_MON_STATE,
    FC_ALREADY_INITIALIZED,
    FC_HANG,
};

void panic(int code);

#define ASSERT(expr, code) { if(!(expr)) {panic(code);} }

uint32_t busy_wait_millis(uint32_t millis);

#endif // __SUPER_H__
