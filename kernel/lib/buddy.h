#ifndef __BUDDY_H__
#define __BUDDY_H__

#include "sys/types.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef struct 
{
    size_t  r_start;
    size_t  r_size;
} Range;

void
buddy_setup(size_t total, Range* hole);

size_t
buddy_alloc(size_t total);

void
buddy_free(size_t begpos, size_t size);

unsigned int
buddy_test(int level, int pos);
#ifdef __cplusplus
}
#endif // __cplusplus

#endif  // __BUDDY_H__