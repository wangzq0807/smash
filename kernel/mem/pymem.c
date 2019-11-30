#include "asm.h"
#include "arch/page.h"
#include "lib/log.h"
#include "lib/bitmap.h"
#include "kerrno.h"
#include "pymem.h"

#define BITMAP_SIZE 1024
bitmap_t pybitmap;
bitmap pybm_struct;
uint8_t bitbuf[BITMAP_SIZE];

void
init_pymemory()
{
    pybm_struct.b_nsize = BITMAP_SIZE;
    pybm_struct.b_bitbuf = &bitbuf;
    pybitmap = &pybm_struct;
}

int
alloc_pyrange(uint32_t rbeg, uint32_t rsize)
{
    const int begbit = rbeg >> PAGE_LOG_SIZE;
    const int bitnum = rsize >> PAGE_LOG_SIZE;
    int hasbit = bm_set_bitrange(pybitmap, begbit, bitnum);
    if (hasbit)
        return ERR_PARAM_ILLEGAL;
    bm_set_bitrange(pybitmap, begbit, bitnum);
    return 0;
}

uint32_t
alloc_pypage()
{
    int nbit = bm_alloc_bit(pybitmap);
    if (nbit < 0)
    {
        KLOG(ERROR, "alloc_pypage failed!");
        return 0;
    }
    else
    {
        return nbit << PAGE_LOG_SIZE;
    }
}

void
release_pypage(uint32_t paddr)
{
    KLOG(DEBUG, "release_pypage %X", paddr);

    int nIndex = paddr >> PAGE_LOG_SIZE;
    bm_clear_bit(pybitmap, nIndex);
}

int
is_pypage_used(uint32_t paddr)
{
    int nIndex = paddr >> PAGE_LOG_SIZE;
    return bm_test_bit(pybitmap, nIndex);
}

void
dump_pymemory()
{
    bm_dump(pybitmap, 0, 1024);
}
