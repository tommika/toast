// Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License
#ifndef __ASSERT__
#define __ASSERT__

void assert_failed(const char * expr, const char * filename, int lineno, int panic_code);

#define ASSERT(expr,panic_code) { if(!(expr)) {assert_failed(#expr,__FILE__,__LINE__,panic_code);}}

#endif // __ASSERT__
