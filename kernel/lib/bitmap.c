#include "bitmap.h"
#include "kerrno.h"
#include "lib/log.h"

typedef uint32_t    bm_unit_t;
#define BM_UNIT_FULL            (0xffffffff)
#define BM_UNIT_BITNUM          (8*sizeof(bm_unit_t))

#define BM_UNIT_INDEX(nbit)     ((nbit) / BM_UNIT_BITNUM)
#define BM_BIT_INDEX(nbit)      ((nbit) % BM_UNIT_BITNUM)

int
bm_test_bit(BitMap* pbitmap, int nbit)
{
    const int unit_index = BM_UNIT_INDEX(nbit);
    const int bit_index = BM_BIT_INDEX(nbit);
    bm_unit_t *bmbuf = (bm_unit_t *)(pbitmap->b_bitbuf);
    if (GET_BIT(bmbuf[unit_index], bit_index))
        return 1;
    else
        return 0;
}

int
bm_set_bit(BitMap* pbitmap, int nbit)
{
    const int unit_index = BM_UNIT_INDEX(nbit);
    const int bit_index = BM_BIT_INDEX(nbit);
    bm_unit_t *bmbuf = (bm_unit_t *)(pbitmap->b_bitbuf);
    SET_BIT(bmbuf[unit_index], bit_index);
    return ERR_SUCCESS;
}

int
bm_clear_bit(BitMap* pbitmap, int nbit)
{
    const int unit_index = BM_UNIT_INDEX(nbit);
    const int bit_index = BM_BIT_INDEX(nbit);
    bm_unit_t *bmbuf = (bm_unit_t *)(pbitmap->b_bitbuf);
    CLEAR_BIT(bmbuf[unit_index], bit_index);
    return ERR_SUCCESS;
}

int
bm_find_bit(BitMap* pbitmap)
{
    int nRet = ERR_MEM_SPACE;
    bm_unit_t *bmbuf = (bm_unit_t *)(pbitmap->b_bitbuf);
    const int endpos = BM_UNIT_INDEX(pbitmap->b_nsize);
    for (int i = 0; i < endpos; ++i)
    {
        if (bmbuf[i] == BM_UNIT_FULL)
            continue;
        int pos = UNIT_FIND_BIT(bm_unit_t, bmbuf[i], 0, BM_UNIT_BITNUM);
        nRet = i*BM_UNIT_BITNUM + pos;
        break;
    }
    return nRet;
}

int
bm_alloc_bit(BitMap* pbitmap)
{
    int nRet = ERR_MEM_SPACE;
    bm_unit_t *bmbuf = (bm_unit_t *)(pbitmap->b_bitbuf);
    const int endpos = BM_UNIT_INDEX(pbitmap->b_nsize);
    for (int i = 0; i < endpos; ++i)
    {
        if (bmbuf[i] == BM_UNIT_FULL)
            continue;
        int pos = UNIT_ALLOC_BIT(bm_unit_t, bmbuf[i], 0, BM_UNIT_BITNUM);
        nRet = i*BM_UNIT_BITNUM + pos;
        break;
    }
    return nRet;
}

int
bm_alloc_bit_inrange(BitMap* pbitmap, const int begbit, const uint32_t bitnum)
{
    if (bitnum == 0)
        return ERR_PARAM_ILLEGAL;
    if (begbit >= (8*pbitmap->b_nsize))
        return ERR_PARAM_ILLEGAL;
    const int endbit = MIN(begbit + bitnum, 8*pbitmap->b_nsize);
    bm_unit_t *bmbuf = (bm_unit_t *)(pbitmap->b_bitbuf);
    const int begpos = BM_UNIT_INDEX(begbit);
    const int endpos = BM_UNIT_INDEX(endbit);
    if (begpos == endpos)
    {
        int ret = UNIT_ALLOC_BIT(bm_unit_t, bmbuf[begpos], BM_BIT_INDEX(begbit), endbit - begbit);
        if (ret != BM_UNIT_FULL)
            return BM_UNIT_BITNUM * begpos + ret;
    }
    else
    {
        int ret = UNIT_ALLOC_BIT(bm_unit_t, bmbuf[begpos], BM_BIT_INDEX(begbit), BM_UNIT_BITNUM - BM_BIT_INDEX(begbit));
        if (ret != BM_UNIT_FULL)
            return BM_UNIT_BITNUM * begpos + ret;

        const int begpos_ceil = (begbit + BM_UNIT_BITNUM - 1) / BM_UNIT_BITNUM;
        const int endpos_floor = endpos;
        for (int i = begpos_ceil; i < endpos_floor; ++i)
        {
            int ret = UNIT_ALLOC_BIT(bm_unit_t, bmbuf[i], 0, BM_UNIT_BITNUM);
            if (ret != BM_UNIT_FULL)
                return i * BM_UNIT_BITNUM + ret;
        }

        ret = UNIT_ALLOC_BIT(bm_unit_t, bmbuf[endpos], 0, BM_BIT_INDEX(endbit));
        if (ret != BM_UNIT_FULL)
            return BM_UNIT_BITNUM * endpos + ret;
    }
    return ERR_MEM_ACCESS;
}

int
bm_test_bitrange(BitMap* pbitmap, const int begbit, const uint32_t bitnum)
{
    if (bitnum == 0)
        return ERR_PARAM_ILLEGAL;
    if (begbit >= (8*pbitmap->b_nsize))
        return ERR_MEM_ACCESS;
    const int endbit = MIN(begbit + bitnum, 8*pbitmap->b_nsize);
    bm_unit_t *bmbuf = (bm_unit_t *)(pbitmap->b_bitbuf);
    const int begpos = BM_UNIT_INDEX(begbit);
    const int endpos = BM_UNIT_INDEX(endbit);
    if (begpos == endpos)
    {
        return UNIT_TEST_BIT(bm_unit_t, bmbuf[begpos], BM_BIT_INDEX(begbit), BM_BIT_INDEX(endbit));
    }

    if (UNIT_TEST_BIT(bm_unit_t, bmbuf[begpos], BM_BIT_INDEX(begbit), BM_UNIT_BITNUM))
        return 1;

    const int begpos_ceil = BM_BIT_INDEX(begbit + BM_UNIT_BITNUM - 1);
    const int endpos_floor = endpos;
    for (int i = begpos_ceil; i < endpos_floor; ++i)
    {
        if (bmbuf[i] != 0)
            return 1;
    }

    if (UNIT_TEST_BIT(bm_unit_t, bmbuf[endpos], 0, BM_BIT_INDEX(endbit)))
        return 1;

    return 0;
}

int
bm_set_bitrange(BitMap* pbitmap, const int begbit, const uint32_t bitnum)
{
    if (bitnum == 0)
        return ERR_PARAM_ILLEGAL;
    if (begbit >= (8*pbitmap->b_nsize))
        return ERR_MEM_ACCESS;
    const int endbit = MIN(begbit + bitnum, 8*pbitmap->b_nsize);
    bm_unit_t *bmbuf = (bm_unit_t *)(pbitmap->b_bitbuf);
    const int begpos = BM_UNIT_INDEX(begbit);
    const int endpos = BM_UNIT_INDEX(endbit);
    if (begpos == endpos)
    {
        UNIT_SET_BIT(bm_unit_t, bmbuf[begpos], BM_BIT_INDEX(begbit), endbit - begbit);
        return 0;
    }

    const int begpos_ceil = BM_UNIT_INDEX(begbit + BM_UNIT_BITNUM - 1);
    const int endpos_floor = endpos;
    for (int i = begpos_ceil; i < endpos_floor; ++i)
        bmbuf[i] = BM_UNIT_FULL;

    UNIT_SET_BIT(bm_unit_t, bmbuf[begpos], BM_BIT_INDEX(begbit), BM_UNIT_BITNUM - BM_BIT_INDEX(begbit));
    UNIT_SET_BIT(bm_unit_t, bmbuf[endpos], 0, BM_BIT_INDEX(endbit));
    return ERR_SUCCESS;
}

int
bm_clear_bitrange(BitMap* pbitmap, const int begbit, const uint32_t bitnum)
{
    if (bitnum == 0)
        return ERR_PARAM_ILLEGAL;
    if (begbit >= (8*pbitmap->b_nsize))
        return ERR_MEM_ACCESS;
    const int endbit = MIN(begbit + bitnum, 8*pbitmap->b_nsize);
    bm_unit_t *bmbuf = (bm_unit_t *)(pbitmap->b_bitbuf);
    const int begpos = BM_UNIT_INDEX(begbit);
    const int endpos = BM_UNIT_INDEX(endbit);
    if (begpos == endpos)
    {
        UNIT_CLEAR_BIT(bm_unit_t, bmbuf[begpos], BM_BIT_INDEX(begbit), endbit - begbit);
        return 0;
    }

    const int begpos_ceil = BM_UNIT_INDEX(begbit + BM_UNIT_BITNUM - 1);
    const int endpos_floor = endpos;
    for (int i = begpos_ceil; i < endpos_floor; ++i)
        bmbuf[i] = 0;

    UNIT_CLEAR_BIT(bm_unit_t, bmbuf[begpos], BM_BIT_INDEX(begbit), BM_UNIT_BITNUM - BM_BIT_INDEX(begbit));
    UNIT_CLEAR_BIT(bm_unit_t, bmbuf[endpos], 0, BM_BIT_INDEX(endbit));
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
    bm_unit_t *bmbuf = (bm_unit_t *)(pbitmap->b_bitbuf);
    const int step = (sizeof(int));
    const int begpos = begbyte / (4*step) * 4;
    const int endpos = (endbyte + step - 1) / step;
    for (int i = begpos; i + 4 < endpos; i+= 4)
    {
        KLOG(ERROR, "bm_dump %d: %X %X %X %X", i*step, bmbuf[i], bmbuf[i+1], bmbuf[i+2], bmbuf[i+3]);
    }
}

