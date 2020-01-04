#include "sys/types.h"
#include "arch/task.h"
#include "arch/irq.h"
#include "arch/page.h"
#include "memory.h"
#include "asm.h"
#include "string.h"
#include "lib/log.h"

static void
setup_new_tss(IrqFrame *irq, Task *new_task)
{
    // 内核态堆栈不能实现写时复制。
    vm_t knl_stack = alloc_vm_page();
    ((uint32_t *)knl_stack)[0] = (uint32_t)new_task;
    /******************************
     * 复制父任务的上下文
     * NOTE: 这里复制的并不是父任务的及时上下文，而是父任务fork返回后的上下文(eax除外)
     * 这样可以保证不会弄脏堆栈
     ******************************/
    X86TSS *new_tss = &new_task->ts_tss;
    new_tss->t_ESP_0 = (uint32_t)(knl_stack + PAGE_SIZE);
    new_tss->t_SS_0 = KNL_DS;

    new_tss->t_CR3 = (uint32_t)alloc_vm_page();
    new_tss->t_EIP = irq->if_EIP;       // 将子任务的eip指向fork调用的下一条指令
    new_tss->t_EFLAGS = irq->if_EFLAGS;
    new_tss->t_EAX = 0;                         // 构造子任务的fork返回值
    new_tss->t_ECX = irq->if_ECX;
    new_tss->t_EDX = irq->if_EDX;
    new_tss->t_EBX = irq->if_EBX;
    new_tss->t_ESP = irq->if_ESP;
    new_tss->t_EBP = irq->if_EBP;
    new_tss->t_ESI = irq->if_ESI;
    new_tss->t_EDI = irq->if_EDI;
    /* 下面对所有任务都是固定值 */
    new_tss->t_ES = USR_DS;
    new_tss->t_CS = USR_CS;
    new_tss->t_SS = USR_DS;
    new_tss->t_DS = USR_DS;
    new_tss->t_FS = USR_DS;
    new_tss->t_GS = USR_DS;
    new_tss->t_LDT = KNL_LDT;
    new_tss->t_Trap = 0;
    new_tss->t_IO_map = 0x8000;
}

static void
setup_page_tables(Task *cur_task, Task *new_task)
{
    /******************************
     * 复制父任务的页表
     * NOTE:下面会就将父任务和新任务的页都设置为只读
     * 但父任务在fork返回前会有很多写内存的操作，
     * 这不会引起写时复制，因为内核态对任何页都是可读写的.
     ******************************/
    pdt_t cur_pdt = (pdt_t)PAGE_FLOOR(cur_task->ts_tss.t_CR3);
    pdt_t new_pdt = (pdt_t)PAGE_FLOOR(new_task->ts_tss.t_CR3);

    // 复制4M - 4G的页表
    // Note: alloc_vm_page 会在1 - 4M空间分配页表，导致1-4M的页表在页表复制过程中改变
    // 因此1-4M的页表要最后复制
    for (int npde = 1; npde < PAGE_ENTRY_NUM; ++npde) {
        if (cur_pdt[npde] & PAGE_PRESENT) {
            pt_t cur_pt = pde2pt(cur_pdt[npde]);
            pt_t new_pt = alloc_page_table(&new_pdt[npde]);
            for (int npte = 0; npte < PAGE_ENTRY_NUM; ++npte) {
                if (cur_pt[npte] & PAGE_PRESENT) {
                    //cur_pt[npte] &= ~PAGE_WRITE;
                    new_pt[npte] = cur_pt[npte] & ~PAGE_WRITE;
                    //add_pypage_refs(cur_pt[npte] & 0xFFFFF000);
                }
            }
        }
    }

    /* 刷新tlb */
    load_pdt(cur_pdt);
}

int
sys_fork(IrqFrame *irq)
{
    Task *cur_task = current_task();
    Task *newtsk = new_task(cur_task);
    setup_new_tss(irq, newtsk);
    setup_page_tables(cur_task, newtsk);

    for (int i = 0; i < MAX_FD; ++i)
        newtsk->ts_filps[i] = dup_vfile(cur_task->ts_filps[i]);
    newtsk->ts_cdev = cur_task->ts_cdev;
    newtsk->ts_cinode = cur_task->ts_cinode;
    newtsk->ts_wait = NULL;
    // 允许新进程被调度执行
    newtsk->ts_state = TS_RUN;

    return newtsk->ts_pid;
}
