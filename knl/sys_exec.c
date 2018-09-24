#include "arch/irq.h"
#include "arch/task.h"
#include "arch/page.h"
#include "asm.h"
#include "elf.h"
#include "log.h"
#include "memory.h"
#include "fs/file.h"
#include "sys/fcntl.h"

#define ELF_FILE       0x40000000

int
sys_execve(IrqFrame *irqframe, char *execfile, char **argv, char **envp)
{
    printk("exec %s\n", execfile);
    IndexNode *fnode = file_open(execfile, O_RDONLY, 0);
    const uint32_t filesize = fnode->in_inode.in_file_size;
    uint32_t sizecnt = 0;
    // 读取elfheader
    uint32_t pyheader = alloc_pypage();
    void *headbuf = (void *)(ELF_FILE);
    map_vm_page(ELF_FILE, pyheader);
    for (int n = 0; n < PAGE_SIZE / BLOCK_SIZE; ++n) {
        file_read(fnode, n * BLOCK_SIZE , headbuf + n * BLOCK_SIZE, BLOCK_SIZE);
    }
    ElfHeader *elfheader = (ElfHeader *)(ELF_FILE);
    printk("elf %x\n", elfheader->eh_magic);
    irqframe->if_EIP = elfheader->eh_entry;
    uint32_t linear = PAGE_FLOOR(elfheader->eh_entry);
    map_vm_page(linear, pyheader);  // TODO:假设entry和elfheader在同一个页面
    if (linear != ELF_FILE) {
        release_vm_page(headbuf);
        // NOTE: 此时elfheader的指针将失效
        elfheader = NULL;
    }
    sizecnt = PAGE_SIZE;
    // 读取剩余文件内容
    pde_t *pdt = (pde_t *)get_cr3();
    while (sizecnt < filesize) {
        uint32_t pyaddr = alloc_pypage();
        void *buf = (void *)(linear + sizecnt);
        map_vm_page(linear + sizecnt, pyaddr);
        for (int n = 0; n < PAGE_SIZE / BLOCK_SIZE; ++n) {
            int rd = file_read( fnode,
                                sizecnt + n * BLOCK_SIZE,
                                buf + n * BLOCK_SIZE,
                                BLOCK_SIZE);
            printk(" rd %x ", rd);
        }
        sizecnt += PAGE_SIZE;
    }

    load_cr3(pdt);

    return 0;
}

static void
_free_task_memory(Task *task)
{
    pde_t *cur_pdt = (pde_t *)PAGE_FLOOR(task->ts_tss.t_CR3);
    for (uint32_t npde = 1; npde < (PAGE_SIZE / sizeof(pde_t)); ++npde) {
        if (cur_pdt[npde] & PAGE_PRESENT) {
            pte_t *pet = (pte_t *)PAGE_FLOOR(cur_pdt[npde]);
            for (uint32_t npte = 0; npte < (PAGE_SIZE / sizeof(pte_t)); ++npte) {
                if (pet[npte] & PAGE_PRESENT)
                    release_pypage(PAGE_FLOOR(pet[npte]));
            }
            cur_pdt[npde] &= ~PAGE_PRESENT;
        }
    }
    // pte_t *pet = (pte_t *)PAGE_FLOOR(cur_pdt[0]);
    // for (uint32_t npte = 0; npte < (PAGE_SIZE / sizeof(pte_t)); ++npte) {
    //     pet[npte] &= ~PAGE_PRESENT;
    // }
    // cur_pdt[0] &= ~PAGE_PRESENT;
}

extern int sys_close(IrqFrame *irq, int fd);

static void
_close_task_files(Task *task)
{
    for (int i = 0; i < task->ts_findex; ++i) {
        sys_close(NULL, i);
    }
}

int
sys_exit(IrqFrame *irq, int code)
{
    Task *cur_task = current_task();
    cur_task->ts_exit = code;
    cur_task->ts_state = TS_ZOMBIE;
    _free_task_memory(cur_task);
    _close_task_files(cur_task);

    switch_task();
    return 0;
}

int
sys_waitpid(IrqFrame *irq, pid_t pid, int *status, int options)
{
    return 0;
}

int
sys_pause(IrqFrame *irq)
{
    switch_task();
    return 0;
}
