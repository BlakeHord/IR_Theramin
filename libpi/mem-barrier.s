// this is the full-on memory barrier --- i think is overkill.
// https://github.com/raspberrypi/firmware/wiki/Accessing-mailboxes
.globl mb
mb:
    mov r0, #0
    mcr p15, 0, r0, c7, c5, 0   // Invalidate instruction cache
    mcr p15, 0, r0, c7, c5, 6   // Invalidate BTB
    mcr p15, 0, r0, c7, c10, 4  // Drain write buffer
    mcr p15, 0, r0, c7, c5, 4   // Prefetch flush
        bx lr

/*
 * Data memory barrier
 *
 * No memory access after the DMB can run until
 * all memory accesses before it have completed
 *
	This memory barrier ensures that all explicit memory transactions
	occurring in program order before this instruction are
	completed. No explicit memory transactions occurring in program
	order after this instruction are started until this instruction
	completes. Other instructions can complete out of order with
	the Data Memory Barrier instruction.
 *
 */
.globl dmb
dmb:
	@ dmb;
	mov r0, #0			// bug: did not have this
        mcr p15, 0, r0, c7, c10, 5
        bx lr

/*
 * Data synchronisation barrier
 *
 * No instruction after the DSB can run until
 * all instructions before it have completed

	This memory barrier completes when all explicit memory
	transactions occurring in program order before this instruction
	are completed. No explicit memory transactions occurring in
	program order after this instruction are started until this
	instruction completes. In fact, no instructions occurring in
	program order after the Data Synchronization Barrier complete,
	or change the interrupt masks, until this instruction completes.

    Note
	This operation has historically been referred to as
	DrainWriteBuffer or DataWriteBarrier (DWB). From ARMv6, these
	names (and the use of DWB) are deprecated in favor of the new
	DataSynchronizationBarrier name and DSB. DSB better reflects the
	functionality provided in ARMv6; it is architecturally defined
	to include all cache, TLB and branch prediction maintenance
	operations as well as explicit memory operations.
 */
.globl dsb
dsb:
	@ dsb;
	mov r0, #0			// bug: did not have this
        mcr p15, 0, r0, c7, c10, 4	// drain write buffer
        bx lr

