#pragma once
#include "sys/types.h"

#ifdef __cplusplus
extern "C" {
#endif
#ifndef pym_t
    typedef unsigned int pym_t;
#endif

void
buddy_setup(size_t total);

pym_t
buddy_alloc(size_t total);

void
buddy_free(pym_t addr, size_t size);

unsigned int
buddy_test(int level, int pos);
#ifdef __cplusplus
}
#endif
