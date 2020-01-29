#include "asm.h"
#include "arch/page.h"
#include "lib/log.h"
#include "lib/bitmap.h"
#include "kerrno.h"
#include "frame.h"
#include "memory.h"
#include "lib/buddy.h"

// TODO: 最大管理16MB物理内存
#define TOTAL_MEM   (16<<20)

typedef struct {
    size_t      fd_addr : 24;
    size_t      fd_ref  : 8;
} FrameDesc;

FrameDesc   frame_nodes[4096];
extern size_t kernel_heap_beg;

void
frame_init()
{
    Range hole[]= {
        { 0xA0000/PAGE_SIZE, 0x100000/PAGE_SIZE }, // 0xA0000 - 1M : BIOS
        { 0, 0 }
    };
    // 挖掉内核代码所占空间
    // hole[0].r_size = kernel_heap_beg/PAGE_SIZE - 0xA0000/PAGE_SIZE;
    buddy_setup(TOTAL_MEM, hole);
}

pym_t
frame_alloc()
{
    int nbit = -1;
    nbit = buddy_alloc(1);

    if (nbit < 0)
    {
        KLOG(ERROR, "frame_alloc failed!");
        return 0;
    }
    else
    {
        frame_nodes[nbit].fd_ref = 1;
        return nbit << PAGE_SHIFT;
    }
}

void
frame_release(pym_t paddr)
{
    int nIndex = paddr >> PAGE_SHIFT;
    int nref = frame_nodes[nIndex].fd_ref;
    if (nref == 0)
        return;
    frame_nodes[nIndex].fd_ref = nref - 1;
    if (nref == 1)
        buddy_free(nIndex, 1);
}

int
frame_add_ref(pym_t paddr)
{
    int nIndex = paddr >> PAGE_SHIFT;
    KLOG(DEBUG, "frame_add_ref %x %d", paddr, frame_nodes[nIndex].fd_ref);
    frame_nodes[nIndex].fd_ref++;
    return 0;
}

int
frame_get_ref(pym_t paddr)
{
    int nIndex = paddr >> PAGE_SHIFT;
    return frame_nodes[nIndex].fd_ref;
}

void
dump_frame_layout()
{
}
