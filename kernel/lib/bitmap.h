#ifndef __BITMAP_H__
#define __BITMAP_H__
#include <sys/types.h>
#include "bithelp.h"
#ifdef __cplusplus
extern "C" {
#endif

//typedef void* BitMap;
typedef struct
{
    int     b_nsize;
    void*   b_bitbuf;
} BitMap;

static inline int
bitmap_get_size(BitMap* pbitmap) {
    return pbitmap->b_nsize;
}

int
bitmap_get_size(BitMap* bm);

int
bitmap_test_bit(BitMap* bm, int nbit);

int
bitmap_set_bit(BitMap* bm, int nbit);

int
bitmap_clear_bit(BitMap* bm, int nbit);

int
bitmap_find_bit(BitMap* bm);

int
bitmap_alloc_bit(BitMap* bm);

int
bitmap_alloc_bit_inrange(BitMap* bm, const int begbit, const uint32_t bitnum);

int
bitmap_test_bitrange(BitMap* bm, const int begbit, const uint32_t bitnum);

int
bitmap_set_bitrange(BitMap* bm, const int begbit, const uint32_t bitnum);

int
bitmap_clear_bitrange(BitMap* bm, const int begbit, const uint32_t bitnum);

void
bitmap_dump(BitMap* bm, const int begbyte, const uint32_t bytenum);

#ifdef __cplusplus
}
#endif

#endif // __BITMAP_H__
