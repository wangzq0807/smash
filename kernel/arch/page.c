#include "page.h"
#include "asm.h"
#include "memory.h"
#include "task.h"
#include "string.h"
#include "lib/log.h"
#include "fs/file.h"

static void on_page_write_protect(vm_t linear, pt_t pt, int npte);
static void on_page_not_exist(vm_t linear, pt_t pt, int npte);

void
on_page_fault(IrqFrame *irq)
{
    vm_t linear = (vm_t)PAGE_FLOOR(get_cr2());
    KLOG(DEBUG, "%s, CR2: %x", __FUNCTION__, get_cr2());
    pt_t pt = get_pt(linear);
    int npte = get_pte_index(linear);
    if (pt[npte] & PAGE_PRESENT)
        on_page_write_protect(linear, pt, npte);
    else
        on_page_not_exist(linear, pt, npte);
    
    invlpg(linear);
}

static void
on_page_write_protect(vm_t linear, pt_t pt, int npte)
{
    //uint32_t pyaddr = pte2pypage(pt[npte]);
    int refs = 0;//get_pypage_refs(pyaddr);
    if (refs > 1) {
        /*
        uint32_t new_page = alloc_pypage();
        pypage_copy(new_page, pyaddr, 1);
        release_pypage(pyaddr); // 减引用计数

        pt[npte] = PAGE_FLOOR((uint32_t)new_page) | PAGE_PRESENT | PAGE_USER | PAGE_WRITE;
        */
    }
    else {
        pt[npte] |= PAGE_WRITE;
    }
}

static void
on_page_not_exist(vm_t linear, pt_t pt, int npte)
{
    Task* ts = current_task();
    int fd = FD_MASK(PAGE_MARK(pt[npte]) >> 1);
    int offset = PAGE_FLOOR(pt[npte]);
    VFile *vf = ts->ts_filps[fd];
    if (fd < MAX_FD && vf != NULL)
    {
        uint32_t pyaddr = alloc_pypage();
        pt[npte] = PAGE_FLOOR((uint32_t)pyaddr) | PAGE_PRESENT | PAGE_USER | PAGE_WRITE;
        file_read(vf->f_inode, offset, (void *)PAGE_FLOOR(linear), PAGE_SIZE);
    }
    else
    {
        KLOG(DEBUG, "page not exist error %x", pt[npte]);
    }
}
