// To keep this in the first portion of the binary.
.section ".text.boot"

.globl _start
_start:

    @  enable fpu
    mrc p15, 0, r0, c1, c0, 2
    orr r0,r0,#0x300000 ;@ single precision
    orr r0,r0,#0xC00000 ;@ double precision
    mcr p15, 0, r0, c1, c0, 2
    mov r0,#0x40000000
    fmxr fpexc,r0

    mov sp, #0x8000000
    mov fp, #0  // i don't think necessary.
    bl _cstart
    bl reboot // if they return just reboot.
