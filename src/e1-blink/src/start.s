@ Copyright (c) 2020, 2024 Thomas Mikalsen. Subject to the MIT License 
.section .init
.globl _start
_start:
@ initialize stack pointer
mov sp, #0x8000
@ branch to main
bl main
@ loop forever
b .

.section .init
.globl yes
yes:
mov r0,#1
mov pc,lr

.globl no
no:
mov r0,#0
mov pc,lr


.globl echo
echo:
mov r5,r0
mov r0,r5
mov pc,lr

.globl not
not:
rsbs  r0, r0, #1
movcc r0, #0
mov pc,lr

