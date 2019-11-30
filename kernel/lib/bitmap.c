#include "bitmap.h"
#include "kerrno.h"
#include "lib/log.h"

int
bm_get_size(bitmap_t bm)
{
    bitmap *pbitmap = (bitmap*)bm;
    return pbitmap->b_nsize;
}

int
bm_test_bit(bitmap_t bm, int nbit)
{
    const int step = 8*sizeof(int);
    const int pos1 = nbit / step;
    const int pos2 = nbit % step;
    bitmap *pbitmap = (bitmap*)bm;
    int *bmbuf = (int *)(pbitmap->b_bitbuf);
    if (_get_bit(bmbuf[pos1], pos2))
        return 1;
    else
        return 0;
}

int
bm_set_bit(bitmap_t bm, int nbit)
{
    const int step = 8*sizeof(int);
    const int pos1 = nbit / step;
    const int pos2 = nbit % step;
    bitmap *pbitmap = (bitmap*)bm;
    int *bmbuf = (int *)(pbitmap->b_bitbuf);
    _set_bit(&bmbuf[pos1], pos2);
    return ERR_SUCCESS;
}

int
bm_clear_bit(bitmap_t bm, int nbit)
{
    const int step = 8*sizeof(int);
    const int pos1 = nbit / step;
    const int pos2 = nbit % step;
    bitmap *pbitmap = (bitmap*)bm;
    int *bmbuf = (int *)(pbitmap->b_bitbuf);
    _clear_bit(&bmbuf[pos1], pos2);
    return ERR_SUCCESS;
}

int
bm_alloc_bit(bitmap_t bm)
{
    int nRet = ERR_MEM_ACCESS;
    bitmap *pbitmap = (bitmap*)bm;
    int *bmbuf = (int *)(pbitmap->b_bitbuf);
    const int endpos = (pbitmap->b_nsize / sizeof(int));
    for (int i = 0; i < endpos; ++i)
    {
        if (bmbuf[i] == -1)
            continue;
        for (int bit = 0; bit < 8*sizeof(int); ++bit)
        {
            if (_get_bit(bmbuf[i], bit) == 0)
            {
                _set_bit(&(bmbuf[i]), bit);
                nRet = i * 8 * sizeof(int) + bit;
                break;
            }
        }
        break;
    }
    return nRet;
}

int
bm_test_bitrange(bitmap_t bm, const int begbit, const int bitnum)
{
    bitmap *pbitmap = (bitmap*)bm;
    const int endbit = begbit + bitnum;
    if (begbit >= (8*pbitmap->b_nsize) ||
        endbit > (8*pbitmap->b_nsize) )
        return ERR_MEM_ACCESS;
    int *bmbuf = (int *)(pbitmap->b_bitbuf);
    const int step = (8*sizeof(int));
    const int begpos = begbit / step;
    const int endpos = endbit / step;
    if (begpos == endpos)
    {
        int bitmask = ((1 << (begbit % step)) - 1) ^ ((1 << (endbit % step)) - 1);
        if (bmbuf[begpos] &= bitmask)
            return 1;
        else
            return 0;
    }
    const int begmask = (-1) & ~ ((1 << (begbit % step)) - 1);
    if (bmbuf[begpos] &= begmask)
        return 1;

    const int begpos_ceil = (begbit + step - 1) / step;
    const int endpos_floor = endpos;
    for (int i = begpos_ceil; i < endpos_floor; ++i)
    {
        if (bmbuf[i] &= -1)
            return 1;
    }

    const int endmask = (1 << (endbit % step)) - 1;
    if (bmbuf[endpos] &= endmask)
        return 1;

    return 0;
}

int
bm_set_bitrange(bitmap_t bm, const int begbit, const int bitnum)
{
    bitmap *pbitmap = (bitmap*)bm;
    const int endbit = begbit + bitnum;
    if (begbit >= (8*pbitmap->b_nsize) ||
        endbit > (8*pbitmap->b_nsize) )
        return ERR_MEM_ACCESS;
    int *bmbuf = (int *)(pbitmap->b_bitbuf);
    const int step = (8*sizeof(int));
    const int begpos = begbit / step;
    const int endpos = endbit / step;
    if (begpos == endpos)
    {
        int bitmask = ((1 << (begbit % step)) - 1) ^ ((1 << (endbit % step)) - 1);
        bmbuf[begpos] |= bitmask;
        return 0;
    }

    const int begpos_ceil = (begbit + step - 1) / step;
    const int endpos_floor = endpos;
    for (int i = begpos_ceil; i < endpos_floor; ++i)
    {
        bmbuf[i] |= -1;
    }

    const int begmask = (-1) & ~ ((1 << (begbit % step)) - 1);
    const int endmask = (1 << (endbit % step)) - 1;
    bmbuf[begpos] |= begmask;
    bmbuf[endpos] |= endmask;
    return ERR_SUCCESS;
}

void
bm_dump(bitmap_t bm, const int begbyte, const int bytenum)
{
    bitmap *pbitmap = (bitmap*)bm;
    const int endbyte = begbyte + bytenum;
    if (begbyte >= pbitmap->b_nsize ||
        endbyte > pbitmap->b_nsize) {
        KLOG(ERROR, "bm_dump error");
        return;
    }
    int *bmbuf = (int *)(pbitmap->b_bitbuf);
    const int step = (sizeof(int));
    const int begpos = begbyte / (4*step) * 4;
    const int endpos = (endbyte + step - 1) / step;
    for (int i = begpos; i + 4 < endpos; i+= 4)
    {
        KLOG(ERROR, "bm_dump %d: %X %X %X %X", i*step, bmbuf[i], bmbuf[i+1], bmbuf[i+2], bmbuf[i+3]);
    }
}

