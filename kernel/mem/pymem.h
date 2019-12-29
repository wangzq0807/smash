#ifndef __PYMEM_H__
#define __PYMEM_H__
#include "sys/types.h"
#include "defs.h"

void
init_pymemory();

uint32_t
alloc_pypage();

void
release_pypage(uint32_t paddr);

int
is_pypage_used(uint32_t paddr);

void
dump_pymemory();

#endif // __PYMEM_H__
