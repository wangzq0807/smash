#ifndef __PYMEM_H__
#define __PYMEM_H__
#include "sys/types.h"

void
init_pymemory();

// 分配一段连续内存, rbeg和rsize必须对齐到4KB
int
alloc_pyrange(uint32_t rbeg, uint32_t rsize);

uint32_t
alloc_pypage();

void
release_pypage(uint32_t paddr);

int
is_pypage_used(uint32_t paddr);

void
dump_pymemory();

#endif // __PYMEM_H__