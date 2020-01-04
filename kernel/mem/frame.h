#ifndef __PYMEM_H__
#define __PYMEM_H__
#include "sys/types.h"
#include "defs.h"

void
frame_init();

size_t
frame_alloc();

void
frame_release(size_t paddr);

int
frame_is_used(size_t paddr);

void
dump_frame_layout();

#endif // __PYMEM_H__
