#include "page.h"
#include "asm.h"
#include "memory.h"
#include "task.h"
#include "string.h"
#include "lib/log.h"
#include "fs/file.h"
#include "mem/frame.h"

static void on_page_not_exist(vm_t linear, pt_t pt, int npte);

void
on_page_fault(IrqFrame *irq)
{
    vm_t linear = (vm_t)PAGE_FLOOR(get_cr2());
    KLOG(DEBUG, "%s, CR2: 0x%x", "on_page_fault", get_cr2());
    pt_t pt = get_pt(linear);
    int npte = get_pte_index(linear);
    if (pt[npte] & PAGE_PRESENT)
        vm_fork_page(linear);
    else
        on_page_not_exist(linear, pt, npte);
    
    invlpg(linear);
}

static void
on_page_not_exist(vm_t linear, pt_t pt, int npte)
{
    Task* ts = current_task();
    int fd = FD_MASK(PAGE_MARK(pt[npte]) >> 1);
    int offset = PAGE_FLOOR(pt[npte]);
    KLOG(DEBUG, "on_page_not_exist %d", fd);
    VFile *vf = ts->ts_filps[fd];
    if (vf != NULL)
    {
        vm_alloc_page(linear);
        file_read(vf->f_inode, offset, (void *)PAGE_FLOOR(linear), PAGE_SIZE);
    }
    else
    {
        KLOG(ERROR, "page not exist error 0x%x", pt[npte]);
    }
}
