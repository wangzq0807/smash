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

//typedef void* BitMap;
typedef struct
{
    int     b_nsize;
    void*   b_bitbuf;
} BitMap;

static inline int
bm_get_size(BitMap* pbitmap) {
    return pbitmap->b_nsize;
}

int
bm_get_size(BitMap* bm);

int
bm_test_bit(BitMap* bm, int nbit);

int
bm_set_bit(BitMap* bm, int nbit);

int
bm_clear_bit(BitMap* bm, int nbit);

int
bm_alloc_bit(BitMap* bm);

int
bm_test_bitrange(BitMap* bm, const int begbit, const int bitnum);

int
bm_set_bitrange(BitMap* bm, const int begbit, const int bitnum);

int
bm_clear_bitrange(BitMap* bm, const int begbit, const int bitnum);

void
bm_dump(BitMap* bm, const int begbyte, const int bytenum);

#ifdef __cplusplus
}
#endif

#endif // __BITMAP_H__
