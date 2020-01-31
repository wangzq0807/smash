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
    task_init_kstack(new_task);
    /******************************
     * 复制父任务的上下文
     * NOTE: 这里复制的并不是父任务的及时上下文，而是父任务fork返回后的上下文(eax除外)
     * 这样可以保证不会弄脏堆栈
     ******************************/
    X86TSS *new_tss = &new_task->ts_tss;
    new_tss->t_CR3 = vm2pym((vm_t)vm_alloc());
    KLOG(DEBUG, "new cr3 0x%x", new_tss->t_CR3);
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
    pdt_t cur_pdt = (pdt_t)pym2vm(PAGE_FLOOR(cur_task->ts_tss.t_CR3));
    pdt_t new_pdt = (pdt_t)pym2vm(PAGE_FLOOR(new_task->ts_tss.t_CR3));

    vm_copy_pagetable(cur_pdt, new_pdt);

    /* 刷新tlb */
    load_pdt(vm2pym((vm_t)cur_pdt));
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
