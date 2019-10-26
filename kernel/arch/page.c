#include "page.h"
#include "asm.h"
#include "memory.h"
#include "task.h"
#include "string.h"
#include "log.h"

void
on_page_fault(IrqFrame *irq)
{
    uint32_t linear = PAGE_FLOOR(get_cr2());
    KLOG(DEBUG, "%s CR2: %x ", __FUNCTION__, get_cr2());
    pt_t pt = get_pt(linear);
    int npte = get_pte_index(linear);
    uint32_t pyaddr = pte2pypage(pt[npte]);
    int refs = get_pypage_refs(pyaddr);
    if (refs > 1) {
        uint32_t new_page = alloc_pypage();
        pypage_copy(new_page, pyaddr, 1);
        release_pypage(pyaddr);

        pt[npte] = PAGE_FLOOR((uint32_t)new_page) | PAGE_PRESENT | PAGE_USER | PAGE_WRITE;
    }
    else {
        pt[npte] |= PAGE_WRITE;
    }

    invlpg(linear);
}