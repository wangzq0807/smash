#include "sys/types.h"
#include "arch/task.h"
#include "arch/irq.h"
#include "arch/page.h"
#include "memory.h"
#include "asm.h"
#include "string.h"
#include "log.h"

pid_t nextpid = 1;

static void
setup_new_tss(IrqFrame *irq, Task *new_task)
{
    // 内核态堆栈不能实现写时复制。
    uint8_t *knl_stack = (uint8_t *)alloc_vm_page();
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
    pde_t *cur_pdt = (pde_t *)PAGE_FLOOR(cur_task->ts_tss.t_CR3);
    pde_t *new_pdt = (pde_t *)PAGE_FLOOR(new_task->ts_tss.t_CR3);

    // 复制4M - 4G的页表
    // Note: alloc_vm_page 会在1 - 4M空间分配页表，导致1-4M的页表在页表复制过程中改变
    // 因此1-4M的页表要最后复制
    for (int npde = 1; npde < (PAGE_SIZE / sizeof(pde_t)); ++npde) {
        if (cur_pdt[npde] & PAGE_PRESENT) {
            pte_t *cur_pte = (pte_t *)PAGE_FLOOR(cur_pdt[npde]);
            pte_t *new_pte = (pte_t *)alloc_vm_page();
            new_pdt[npde] = PAGE_FLOOR((uint32_t)new_pte) | PAGE_WRITE | PAGE_USER | PAGE_PRESENT;
            for (int npte = 0; npte < (PAGE_SIZE / sizeof(pte_t)); ++npte) {
                if (cur_pte[npte] & PAGE_PRESENT) {
                    cur_pte[npte] &= ~PAGE_WRITE;
                    new_pte[npte] = cur_pte[npte];
                    add_pypage_refs(cur_pte[npte] & 0xFFFFF000);
                }
            }
        }
    }
    // 复制内核所在的0 - 4M页表
    pte_t *cur_pte = (pte_t *)PAGE_FLOOR(cur_pdt[0]);
    pte_t *new_pte = (pte_t *)alloc_vm_page();
    new_pdt[0] = PAGE_FLOOR((uint32_t)new_pte) | PAGE_WRITE | PAGE_USER | PAGE_PRESENT;
    for (int npte = 0; npte < (PAGE_SIZE / sizeof(pte_t)); ++npte) {
        if (cur_pte[npte] & PAGE_PRESENT) {
            new_pte[npte] = cur_pte[npte];  
        }
    }
    /* 刷新tlb */
    load_cr3(cur_pdt);
}

static void
setup_links(Task *cur_task, Task *new_task)
{
    new_task->ts_parent = cur_task;
    new_task->ts_child_new = NULL;
    new_task->ts_child_old = NULL;
    if (cur_task->ts_child_new != NULL)
        new_task->ts_older = cur_task->ts_child_new;
    else
        new_task->ts_older = cur_task;
    new_task->ts_newer = cur_task;

    // NOTE: 下面的操作未完成前，不能发生进程切换
    cli();
    if (cur_task->ts_child_old == NULL)
        cur_task->ts_child_old = new_task;
    if (cur_task->ts_child_new != NULL) {
        cur_task->ts_child_new->ts_newer = new_task;
    }
    cur_task->ts_child_new = new_task;
    sti();
}

int
sys_fork(IrqFrame *irq)
{
    Task *cur_task = current_task();
    Task *new_task = (Task *)alloc_vm_page();
    new_task->ts_pid = nextpid++;
    setup_new_tss(irq, new_task);
    setup_page_tables(cur_task, new_task);
    setup_links(cur_task, new_task);

    return new_task->ts_pid;
}
