// Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License
#ifndef __SWI_OPS__
#define __SWI_OPS__

// SWI operator values (24-bit)
#define SWI_SET_LED      0x0001
#define SWI_BUSYWAIT     0x0002

// Blocking operations
#define SWI_BLOCKING     0x8000
#define SWI_YIELD        0x8001

#define SWI_MASK         0xFF000000

#endif // __SWI_OPS__
