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
    uint32_t npdt = linear >> 22;
    uint32_t npte = (linear >> 12) & 0x3FF;

    pde_t *pdt = (pde_t *)get_cr3();
    pte_t *pte = (pte_t *)(pdt[npdt] & 0xFFFFF000);

    int pyaddr = pte[npdt] & 0xFFFFF000;
    int refs = get_pypage_refs(pyaddr);
    if (refs > 1) {
        uint32_t new_page = alloc_pypage();
        pypage_copy(new_page, pyaddr, 1);
        release_pypage(pyaddr);

        pte[npte] = PAGE_FLOOR((uint32_t)new_page) | PAGE_PRESENT | PAGE_USER | PAGE_WRITE;
    }
    else {
        pte[npte] = pte[npte] | PAGE_WRITE;
    }

    invlpg((void *)linear);
}