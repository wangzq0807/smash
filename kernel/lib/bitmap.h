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
bm_alloc_bit_inrange(BitMap* bm, const int begbit, const uint32_t bitnum);

int
bm_test_bitrange(BitMap* bm, const int begbit, const uint32_t bitnum);

int
bm_set_bitrange(BitMap* bm, const int begbit, const uint32_t bitnum);

int
bm_clear_bitrange(BitMap* bm, const int begbit, const uint32_t bitnum);

void
bm_dump(BitMap* bm, const int begbyte, const uint32_t bytenum);

#ifdef __cplusplus
}
#endif

#endif // __BITMAP_H__
