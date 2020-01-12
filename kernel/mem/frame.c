#include "asm.h"
#include "arch/page.h"
#include "lib/log.h"
#include "lib/bitmap.h"
#include "kerrno.h"
#include "frame.h"
#include "memory.h"

// TODO: 最大管理64MB物理内存
#define BITMAP_SIZE 2048

BitMap pybitmap;
uint8_t bitbuf[BITMAP_SIZE];

extern vm_t knl_space_begin;

static error_t
_frame_alloc_range(uint32_t rbeg, uint32_t rsize);

void
frame_init()
{
    pybitmap.b_nsize = BITMAP_SIZE;
    pybitmap.b_bitbuf = &bitbuf;
    // 0xA0000 - 1M : BIOS
    _frame_alloc_range(0xA0000, 0x60000);
    // 1M -2M : 内核代码
    _frame_alloc_range(1 << 20, 1 << 20);
}

// 分配一段连续内存, rbeg和rsize必须对齐到4KB
static error_t
_frame_alloc_range(uint32_t rbeg, uint32_t rsize)
{
    const int begbit = rbeg >> PAGE_SHIFT;
    const int bitnum = rsize >> PAGE_SHIFT;
    int hasbit = bitmap_set_bitrange(&pybitmap, begbit, bitnum);
    if (hasbit)
        return ERR_PARAM_ILLEGAL;
    bitmap_set_bitrange(&pybitmap, begbit, bitnum);
    return ERR_SUCCESS;
}

pym_t
frame_alloc()
{
    int nbit = -1;
    nbit = bitmap_alloc_bit(&pybitmap);
    KLOG(DEBUG, "frame_alloc %x", nbit << PAGE_SHIFT);

    if (nbit < 0)
    {
        KLOG(ERROR, "frame_alloc failed!");
        return 0;
    }
    else
    {
        return nbit << PAGE_SHIFT;
    }
}

void
frame_release(pym_t paddr)
{
    KLOG(DEBUG, "release_pypage %X", paddr);

    int nIndex = paddr >> PAGE_SHIFT;
    if (bitmap_test_bit(&pybitmap, nIndex))
        bitmap_clear_bit(&pybitmap, nIndex);
    else
        KLOG(ERROR, "release_pypage ERROR!");
}

int
frame_is_used(pym_t paddr)
{
    int nIndex = paddr >> PAGE_SHIFT;
    return bitmap_test_bit(&pybitmap, nIndex);
}

void
dump_frame_layout()
{
    bitmap_dump(&pybitmap, 0, 1024);
}
