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

#define UNIT_SET_BIT(unit_t, target, beg, num)      \
({\
    const unit_t begmask = ((unit_t)1 << beg) - 1;\
    const unit_t endmask = ((((unit_t)1<< (beg+num-1)) - 1) << 1) + 1; \
    unit_t mask = endmask ^ begmask;                \
    target |= mask;                                 \
})

#define UNIT_CLEAR_BIT(unit_t, target, beg, num)    \
({\
    const unit_t begmask = ((unit_t)1 << beg) - 1;\
    const unit_t endmask = ((((unit_t)1<< (beg+num-1)) - 1) << 1) + 1; \
    unit_t mask = endmask ^ begmask;                \
    target &= ~mask;                                \
})

#define UNIT_GET_BIT(unit_t, target, beg, num)  \
({\
    const unit_t tmp = target >> beg;           \
    const unit_t mask = ((((unit_t)1 << (num-1)) - 1) << 1) + 1; \
    tmp & mask;                                 \
})

#define UNIT_TEST_BIT(unit_t, target, beg, num) \
({\
    const unit_t tmp = target >> beg;           \
    const unit_t mask = ((((unit_t)1<< (num-1)) - 1) << 1) + 1; \
    int nret = 0;                               \
    if ((tmp & mask) != 0)                   \
        nret = 1;                               \
    nret;                                       \
})

#define UNIT_FIND_BIT(unit_t, target, beg, num)\
({\
    int nret = -1;                              \
    int end = MIN(beg+num, 8*sizeof(unit_t));   \
    for (int bit = beg; bit < end; ++bit) {     \
        const unit_t mask = (unit_t)1 << bit;   \
        if ((target & mask) == 0) {             \
            nret = bit;                         \
            break;                              \
        }                                       \
    }                                           \
    nret;                                       \
})

#define UNIT_ALLOC_BIT(unit_t, target, beg, num)\
({\
    int nret = UNIT_FIND_BIT(unit_t, target, beg, num);\
    if (nret != -1)                             \
        target |= 1 << nret;                    \
    nret;                                       \
})

#ifdef __cplusplus
}
#endif

#endif // __BITHELP_H__
