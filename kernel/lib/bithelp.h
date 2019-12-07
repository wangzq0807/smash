#ifndef __BITHELP_H__
#define __BITHELP_H__

#include <stdint.h>
#include "defs.h"
#ifdef __cplusplus
extern "C" {
#endif

#define SET_BIT(val, bit) { \
    val |= 1 << bit;        \
}

#define GET_BIT(val, bit) ({    \
    int tmp = (val >> bit) & 1; \
    tmp;                        \
})

#define CLEAR_BIT(val, bit) {   \
    val &= ~(1<<bit);           \
}

static inline void
set_bit32(uint32_t *target, int beg, int num)
{
    const uint32_t begmask = (1 << (beg)) - 1;
    const uint32_t endmask = (1 << (beg+num)) - 1;
    uint32_t mask = endmask ^ begmask;
    *target |= mask;
}

static inline void
clear_bit32(uint32_t *target, int beg, int num)
{
    const uint32_t begmask = (1 << (beg)) - 1;
    const uint32_t endmask = (1 << (beg+num)) - 1;
    uint32_t mask = endmask ^ begmask;
    *target &= ~mask;
}

static inline int
get_bit32(uint32_t target, int beg, int num)
{
    target = target >> beg;
    const uint32_t mask = (1 << num) - 1;
    return target & mask;
}

static inline int
test_bit32(uint32_t target, int beg, int num)
{
    target = target >> beg;
    const uint32_t mask = (1 << num) - 1;
    if ((target & mask) == 0)
        return 0;
    else
        return 1;
}

inline int
alloc_bit32(uint32_t *target, const int beg, const int num)
{
    int end = MIN(beg+num, 32);
    for (int bit = beg; bit < end; ++bit)
    {
        const uint32_t mask = 1 << bit;
        if ((*target & mask) == 0)
        {
            *target |= mask;
            return bit;
        }
    }
    return -1;
}

#ifdef __cplusplus
}
#endif

#endif // __BITHELP_H__
