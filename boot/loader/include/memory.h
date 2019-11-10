#ifndef __MEMORY_H__
#define __MEMORY_H__
#include "sys/types.h"

void init_memory();

void map_mem(const uint32_t src, const uint32_t dst, uint32_t size);

#endif // __MEMORY_H__
