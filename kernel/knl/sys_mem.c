#include "memory.h"
#include "arch/irq.h"
#include "arch/task.h"
#include "arch/page.h"
#include "log.h"
#include "fs/file.h"
#include "fs/vfile.h"

vm_t
sys_mmap(IrqFrame *irq, vm_t addr, size_t length, int prot, int flags,
                  int fd, off_t offset)
{
    if (PAGE_MARK(addr) > 0)
        return -1;
    pdt_t pdt = get_pdt();
    int npde = get_pde_index(addr);
    pt_t pt = pde2pt(pdt[npde]);
    int npte = get_pte_index(addr);
    if (pt[npte] & PAGE_PRESENT)
        return -1;
    // 所需映射的总页面数
    const int npage = (length + PAGE_SIZE - 1) >> PAGE_LOG_SIZE;
    // TODO: 失败后,要回收已映射的页表项
    // 当前页表的映射 pte
    int lessnum = PAGE_INT_SIZE - npte;
    lessnum = npage > lessnum ? lessnum : npage;
    for (int n = npte; n < lessnum; ++n)
    {
        pt[n] = PAGE_FLOOR(offset) | (fd << 1);
        offset += PAGE_SIZE;
    }
    if (npage <= lessnum)
        return addr;
    npde++;
    // 页目录表的映射
    const int pdenum = npde + (npage - lessnum) / PAGE_INT_SIZE;
    for (int n = npde; n < pdenum; ++n)
    {
        if (pdt[n] & PAGE_PRESENT)
        {
            vm_t pyaddr = alloc_vm_page();
            pdt[n] = PAGE_FLOOR(pyaddr) | PAGE_PRESENT | PAGE_USER | PAGE_WRITE;
        }
        pt = pde2pt(pdt[n]);
        for (int nn = 0; nn < PAGE_INT_SIZE; ++nn)
        {
            pt[nn] = PAGE_FLOOR(offset) | (fd << 1);
            offset += PAGE_SIZE;
        }
    }
    npde += pdenum;
    // 剩余页表的映射 pte
    const int morenum = npage - lessnum - (pdenum*PAGE_INT_SIZE);
    if (pdt[npde] & PAGE_PRESENT)
    {
        vm_t pyaddr = alloc_vm_page();
        pdt[npde] = PAGE_FLOOR(pyaddr) | PAGE_PRESENT | PAGE_USER | PAGE_WRITE;
    }
    pt = pde2pt(pdt[npde]);
    for (int n = 0; n < morenum; ++n)
    {
        pt[n] = PAGE_FLOOR(offset) | (fd << 1);
        offset += PAGE_SIZE;
    }

    return addr;
}
