#include "bitmap.h"
#include "kerrno.h"
#include "lib/log.h"

int
bm_test_bit(BitMap* pbitmap, int nbit)
{
    const int step = 8*sizeof(int);
    const int pos1 = nbit / step;
    const int pos2 = nbit % step;
    int *bmbuf = (int *)(pbitmap->b_bitbuf);
    if (GET_BIT(bmbuf[pos1], pos2))
        return 1;
    else
        return 0;
}

int
bm_set_bit(BitMap* pbitmap, int nbit)
{
    const int step = 8*sizeof(int);
    const int pos1 = nbit / step;
    const int pos2 = nbit % step;
    int *bmbuf = (int *)(pbitmap->b_bitbuf);
    SET_BIT(bmbuf[pos1], pos2);
    return ERR_SUCCESS;
}

int
bm_clear_bit(BitMap* pbitmap, int nbit)
{
    const int step = 8*sizeof(int);
    const int pos1 = nbit / step;
    const int pos2 = nbit % step;
    int *bmbuf = (int *)(pbitmap->b_bitbuf);
    CLEAR_BIT(bmbuf[pos1], pos2);
    return ERR_SUCCESS;
}

int
bm_alloc_bit(BitMap* pbitmap)
{
    int nRet = ERR_MEM_ACCESS;
    uint32_t *bmbuf = (uint32_t *)(pbitmap->b_bitbuf);
    const int endpos = (pbitmap->b_nsize / sizeof(int));
    for (int i = 0; i < endpos; ++i)
    {
        if (bmbuf[i] == -1)
            continue;
        int ret = alloc_bit32(&bmbuf[i], 0, 32);
        if (ret >= 0)
            return i*sizeof(uint32_t)*8 + ret;
    }
    return nRet;
}

int
bm_alloc_bit_inrange(BitMap* pbitmap, const int begbit, const uint32_t bitnum)
{
    if (begbit >= (8*pbitmap->b_nsize))
        return ERR_MEM_ACCESS;
    const int endbit = MIN(begbit + bitnum, 8*pbitmap->b_nsize);
    uint32_t *bmbuf = (uint32_t *)(pbitmap->b_bitbuf);
    const int step = (8*sizeof(uint32_t));
    const int begpos = begbit / step;
    const int endpos = endbit / step;
    if (begpos == endpos)
    {
        int ret = alloc_bit32(&bmbuf[begpos], begbit%step, endbit - begbit);
        if (ret > 0)
            return step * begpos + ret;
    }
    return 0;
}

int
bm_test_bitrange(BitMap* pbitmap, const int begbit, const uint32_t bitnum)
{
    if (begbit >= (8*pbitmap->b_nsize))
        return ERR_MEM_ACCESS;
    const int endbit = MIN(begbit + bitnum, 8*pbitmap->b_nsize);
    int *bmbuf = (int *)(pbitmap->b_bitbuf);
    const int step = (8*sizeof(int));
    const int begpos = begbit / step;
    const int endpos = endbit / step;
    if (begpos == endpos)
    {
        return test_bit32(bmbuf[begpos], begbit % step, endbit % step);
    }

    if (test_bit32(bmbuf[begpos], begbit % step, step - 1))
        return 1;

    const int begpos_ceil = (begbit + step - 1) / step;
    const int endpos_floor = endpos;
    for (int i = begpos_ceil; i < endpos_floor; ++i)
    {
        if (bmbuf[i] != 0)
            return 1;
    }

    if (test_bit32(bmbuf[endpos], 0, endbit % step))
        return 1;

    return 0;
}

int
bm_set_bitrange(BitMap* pbitmap, const int begbit, const uint32_t bitnum)
{
    if (begbit >= (8*pbitmap->b_nsize))
        return ERR_MEM_ACCESS;
    const int endbit = MIN(begbit + bitnum, 8*pbitmap->b_nsize);
    uint32_t *bmbuf = (uint32_t *)(pbitmap->b_bitbuf);
    const int step = (8*sizeof(uint32_t));
    const int begpos = begbit / step;
    const int endpos = endbit / step;
    if (begpos == endpos)
    {
        set_bit32(&bmbuf[begpos], begbit % step, endbit - begbit);
        return 0;
    }

    const int begpos_ceil = (begbit + step - 1) / step;
    const int endpos_floor = endpos;
    for (int i = begpos_ceil; i < endpos_floor; ++i)
    {
        bmbuf[i] = -1;
    }

    set_bit32(&bmbuf[begpos], begbit % step, step - (begbit % step));
    set_bit32(&bmbuf[endpos], 0, endbit % step);
    return ERR_SUCCESS;
}

int
bm_clear_bitrange(BitMap* pbitmap, const int begbit, const uint32_t bitnum)
{
    if (begbit >= (8*pbitmap->b_nsize))
        return ERR_MEM_ACCESS;
    const int endbit = MIN(begbit + bitnum, 8*pbitmap->b_nsize);
    uint32_t *bmbuf = (uint32_t *)(pbitmap->b_bitbuf);
    const int step = (8*sizeof(uint32_t));
    const int begpos = begbit / step;
    const int endpos = endbit / step;
    if (begpos == endpos)
    {
        clear_bit32(&bmbuf[begpos], begbit % step, endbit - begbit);
        return 0;
    }

    const int begpos_ceil = (begbit + step - 1) / step;
    const int endpos_floor = endpos;
    for (int i = begpos_ceil; i < endpos_floor; ++i)
    {
        bmbuf[i] = 0;
    }

    clear_bit32(&bmbuf[begpos], begbit % step, step - begbit % step);
    clear_bit32(&bmbuf[endpos], 0, endbit % step);
    return 0;
}

void
bm_dump(BitMap* pbitmap, const int begbyte, const uint32_t bytenum)
{
    if (begbyte >= pbitmap->b_nsize) {
        KLOG(ERROR, "bm_dump error");
        return;
    }
    const int endbyte = MIN(begbyte + bytenum, pbitmap->b_nsize);
    int *bmbuf = (int *)(pbitmap->b_bitbuf);
    const int step = (sizeof(int));
    const int begpos = begbyte / (4*step) * 4;
    const int endpos = (endbyte + step - 1) / step;
    for (int i = begpos; i + 4 < endpos; i+= 4)
    {
        KLOG(ERROR, "bm_dump %d: %X %X %X %X", i*step, bmbuf[i], bmbuf[i+1], bmbuf[i+2], bmbuf[i+3]);
    }
}

