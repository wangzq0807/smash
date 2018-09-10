#include "arch/irq.h"
#include "arch/task.h"
#include "arch/page.h"
#include "asm.h"
#include "elf.h"
#include "log.h"
#include "memory.h"

#define ELF_FILE        (0x7c00 + 20*1024)

int
sys_execve(IrqFrame *irqframe)
{
    ElfHeader *elfheader = (ElfHeader *)(ELF_FILE);
    // ProgHeader *progheader = (ProgHeader *)(ELF_FILE + elfheader->eh_prog_header);

    Task *cur_task = current_task();
    pde_t *pdt = (pde_t *)cur_task->ts_tss.t_CR3;
    uint32_t npdt = elfheader->eh_entry >> 22;
    uint32_t npte = (elfheader->eh_entry >> 12) & 0x3FF;

    if ( (pdt[npdt] & PAGE_PRESENT) == 0) {
        uint32_t new_page = (uint32_t)alloc_page();
        pdt[npdt] = PAGE_FLOOR(new_page) | PAGE_WRITE | PAGE_USER | PAGE_PRESENT;
    }
    pte_t *pte = (pte_t *)(pdt[npdt] & 0xFFFFF000);
    pte[npte] = PAGE_FLOOR(ELF_FILE) | PAGE_WRITE | PAGE_USER | PAGE_PRESENT;
    irqframe->if_EIP = elfheader->eh_entry;
    load_cr3(pdt);

    return 0;
}


