#ifndef __MEMORY_H__
#define __MEMORY_H__
#include "sys/types.h"

#define PAGE_SHIFT   12
#define PAGE_SIZE     (1 << PAGE_SHIFT)
#define PAGE_INT_SIZE         (PAGE_SIZE/sizeof(int))
#define PAGE_CEILING(addr)    (((addr) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define PAGE_FLOOR(addr)      ((addr) & ~(PAGE_SIZE - 1))
#define PAGE_MARK(addr)         ((addr) & (PAGE_SIZE - 1))


void init_memory();

void map_mem(const vm_t src, const vm_t dst, uint32_t size);

void* alloc_page();

#endif // __MEMORY_H__
