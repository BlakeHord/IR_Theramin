
.globl dummy
dummy:
    bx lr

// https://github.com/raspberrypi/firmware/wiki/Accessing-mailboxes
.globl mb
mb:
    mov r0, #0
    mcr p15, 0, r0, c7, c5, 0   // Invalidate instruction cache
    mcr p15, 0, r0, c7, c5, 6   // Invalidate BTB
    mcr p15, 0, r0, c7, c10, 4  // Drain write buffer
    mcr p15, 0, r0, c7, c5, 4   // Prefetch flush
        bx lr

.globl dmb
dmb:
        mcr p15, 0, r0, c7, c10, 5
        bx lr


.globl enable_fp
enable_fp:
    @  enable fpu
    mrc p15, 0, r0, c1, c0, 2
    orr r0,r0,#0x300000 ;@ single precision
    orr r0,r0,#0xC00000 ;@ double precision
    mcr p15, 0, r0, c1, c0, 2
    mov r0,#0x40000000
    fmxr fpexc,r0
    bx lr
