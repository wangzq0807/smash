#ifndef __PYMEM_H__
#define __PYMEM_H__
#include "sys/types.h"
#include "defs.h"

typedef struct {
    int     fm_magic:4;
    int     fm_fd:8;
    int     fm_offset:20;
} FrameMapDesc;

#define FRAME_MAGIC     (0xA)

void
frame_init();

pym_t
frame_alloc();

void
frame_release(pym_t paddr);

int
frame_is_used(pym_t paddr);

int
frame_add_ref(pym_t paddr);

int
frame_get_ref(pym_t paddr);

void
dump_frame_layout();

#endif // __PYMEM_H__
