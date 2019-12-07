#include "asm.h"
#include "arch/page.h"
#include "lib/log.h"
#include "lib/bitmap.h"
#include "kerrno.h"
#include "pymem.h"

#define BITMAP_SIZE 1024

BitMap pybitmap;
uint8_t bitbuf[BITMAP_SIZE];    // 32MB

extern vm_t knl_space_begin;

void
init_pymemory()
{
    pybitmap.b_nsize = BITMAP_SIZE;
    pybitmap.b_bitbuf = &bitbuf;
    // 1页内核栈和2页页表
    alloc_pyrange(0x0, 3*PAGE_SIZE);
    // 0xA0000 - 1M : BIOS
    alloc_pyrange(0xA0000, 0x60000);
    // 1M -2M : 内核代码
    alloc_pyrange(1 << 20, 1 << 20);
    // 2M - 32M : 内核数据 + 用户空间
}

error_t
alloc_pyrange(uint32_t rbeg, uint32_t rsize)
{
    const int begbit = rbeg >> PAGE_LOG_SIZE;
    const int bitnum = rsize >> PAGE_LOG_SIZE;
    int hasbit = bm_set_bitrange(&pybitmap, begbit, bitnum);
    if (hasbit)
        return ERR_PARAM_ILLEGAL;
    bm_set_bitrange(&pybitmap, begbit, bitnum);
    return ERR_SUCCESS;
}

uint32_t
alloc_pypage(BOOL bKnl)
{
    int nbit = -1;
    if (bKnl)
        nbit = bm_alloc_bit_inrange(&pybitmap, 1<<20, 32<<20);
    else
        nbit = bm_alloc_bit(&pybitmap);

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
    bm_clear_bit(&pybitmap, nIndex);
}

int
is_pypage_used(uint32_t paddr)
{
    int nIndex = paddr >> PAGE_LOG_SIZE;
    return bm_test_bit(&pybitmap, nIndex);
}

void
dump_pymemory()
{
    bm_dump(&pybitmap, 0, 1024);
}
