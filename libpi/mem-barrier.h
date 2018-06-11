#ifndef __MEM_BARRIER_H__
#define __MEM_BARRIER_H__

void mb(void);
void dmb(void);
void dsb(void);

#define read_barrier() dsb()
#define write_barrier() dmb()

#define mem_barrier() do { read_barrier();  write_barrier(); } while(0)
#endif
