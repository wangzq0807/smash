#ifndef __BITMAP_H__
#define __BITMAP_H__
#include "sys/types.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline void
_set_bit(int *byte, int num)
{
    const int val = 1 << num;
    *byte |= val;
}

static inline void
_clear_bit(int *byte, int num)
{
    const int val = 1 << num;
    *byte &= ~val;
}

static inline int
_get_bit(int byte, int num)
{
    const int val = 1 << num;
    return byte & val;
}

typedef void* bitmap_t;
typedef struct
{
    int     b_nsize;
    void*   b_bitbuf;
} bitmap;

int
bm_get_size(bitmap_t bm);

int
bm_test_bit(bitmap_t bm, int nbit);

int
bm_set_bit(bitmap_t bm, int nbit);

int
bm_clear_bit(bitmap_t bm, int nbit);

int
bm_alloc_bit(bitmap_t bm);

int
bm_test_bitrange(bitmap_t bm, const int begbit, const int bitnum);

int
bm_set_bitrange(bitmap_t bm, const int begbit, const int bitnum);

int
bm_clear_bitrange(bitmap_t bm, const int begbit, const int bitnum);

void
bm_dump(bitmap_t bm, const int begbyte, const int bytenum);

#ifdef __cplusplus
}
#endif

#endif // __BITMAP_H__
