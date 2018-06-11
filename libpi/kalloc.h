#ifndef __KALLOC_H__
#define __KALLOC_H__

void *kalloc(unsigned sz);
void kfree_all(void);

void *kalloc_heap_end(void);
void *kalloc_heap_start(void);
#endif /* __KALLOC_H__ */
