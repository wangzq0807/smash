#include "asm.h"
#include "arch/page.h"
#include "lib/log.h"
#include "lib/bitmap.h"
#include "kerrno.h"
#include "pymem.h"

#define BITMAP_SIZE 2048

BitMap pybitmap;
uint8_t bitbuf[BITMAP_SIZE];    // TODO: 最大管理64MB

extern vm_t knl_space_begin;

error_t
_alloc_pyrange(uint32_t rbeg, uint32_t rsize);

void
init_pymemory()
{
    pybitmap.b_nsize = BITMAP_SIZE;
    pybitmap.b_bitbuf = &bitbuf;
    // 0xA0000 - 1M : BIOS
    _alloc_pyrange(0xA0000, 0x60000);
    // 1M -2M : 内核代码
    _alloc_pyrange(1 << 20, 1 << 20);
}

// 分配一段连续内存, rbeg和rsize必须对齐到4KB
error_t
_alloc_pyrange(uint32_t rbeg, uint32_t rsize)
{
    const int begbit = rbeg >> PAGE_SHIFT;
    const int bitnum = rsize >> PAGE_SHIFT;
    int hasbit = bm_set_bitrange(&pybitmap, begbit, bitnum);
    if (hasbit)
        return ERR_PARAM_ILLEGAL;
    bm_set_bitrange(&pybitmap, begbit, bitnum);
    return ERR_SUCCESS;
}

uint32_t
alloc_pypage()
{
    int nbit = -1;
    nbit = bm_alloc_bit(&pybitmap);

    if (nbit < 0)
    {
        KLOG(ERROR, "alloc_pypage failed!");
        return 0;
    }
    else
    {
        return nbit << PAGE_SHIFT;
    }
}

void
release_pypage(uint32_t paddr)
{
    KLOG(DEBUG, "release_pypage %X", paddr);

    int nIndex = paddr >> PAGE_SHIFT;
    if (bm_test_bit(&pybitmap, nIndex))
        bm_clear_bit(&pybitmap, nIndex);
    else
        KLOG(ERROR, "release_pypage ERROR!");
}

int
is_pypage_used(uint32_t paddr)
{
    int nIndex = paddr >> PAGE_SHIFT;
    return bm_test_bit(&pybitmap, nIndex);
}

void
dump_pymemory()
{
    bm_dump(&pybitmap, 0, 1024);
}
