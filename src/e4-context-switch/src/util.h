// Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License
#ifndef __UTIL_H__
#define __UTIL_H__

#define NOINLINE __attribute__((noinline))

void busywait(unsigned int n);

#define FC_ALREADY_INITIALIZED 1
#define FC_OUT_OF_MEM 2
#define FC_EMPTY_QUEUE 3
#define FC_INVALID_PROC_STATE 4
#define FC_INVALID_PROC_MAGIC 5
#define FC_WRONG_PROCESS 6
#define FC_NO_PROCESS 7

void panic(int code);

#define ASSERT(expr,code) { if(!(expr)) {panic(code);} }

#endif // __UTIL_H__
