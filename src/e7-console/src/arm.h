// Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License
#ifndef __ARM_H__
#define __ARM_H__

// See ARM ARM Section A2.5 Program status registers
// A2.5.7 Modes Bits M[4:0]
#define CPSR_MODE_USR       0b00010000  // User mode
#define CPSR_MODE_FIQ       0b00010001  // FIQ mode
#define CPSR_MODE_IRQ       0b00010010  // IRQ mode
#define CPSR_MODE_SVC       0b00010011  // Supervisor mode
#define CPSR_MODE_ABT       0b00010111  // Abort mode
#define CPSR_MODE_UND       0b00011011  // Undefined mode
#define CPSR_MODE_SYS       0b00011111  // System mode
// A2.5.6 The interrupt disable bits (F & I bits)
#define CPSR_DISABLE_FIQ    0b01000000  // Disable FIQ
#define CPSR_DISABLE_IRQ    0b10000000  // Disable IRQ    

// Registers
#define R_R0 0
#define R_R1 1
#define R_SP 13
#define R_LR 14

#endif // __ARM_H__
