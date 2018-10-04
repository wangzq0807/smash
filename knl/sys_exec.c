#include "arch/irq.h"
#include "arch/task.h"
#include "arch/page.h"
#include "asm.h"
#include "elf.h"
#include "log.h"
#include "memory.h"
#include "fs/file.h"
#include "fs/path.h"
#include "sys/fcntl.h"
#include "string.h"

#define ELF_FILE       0x40000000
uint32_t param_buf[64];

static void _free_task_memory(Task *task);

int
sys_execve(IrqFrame *irqframe, const char *execfile, const char **argv, char **envp)
{
    IndexNode *fnode = file_open(execfile, O_RDONLY, 0);
    if (fnode == NULL)  return -1;
    // NOTE:先做参数拷贝后释放内存
    int argc = 0;
    int argsz = 0;
    // 将参数拷贝到内核
    while (argv != NULL && argv[argc] != NULL) {
        int slen = strlen(argv[argc]) + 1;
        int numlen = (slen + sizeof(int) - 1) / sizeof(int);
        param_buf[argsz++] = numlen;
        memcpy(&(param_buf[argsz]), argv[argc], slen);
        argsz += numlen;
        ++argc;
    }

    const uint32_t filesize = fnode->in_inode.in_file_size;
    // 释放当前进程的所有打开文件和物理内存
    Task *curtask = current_task();
    for (int i = 1; i < MAX_FD; ++i) {
        VFile *vf = curtask->ts_filps[i];
        if (vf != NULL) {
            file_close(vf->f_inode);
            release_vfile(vf);
        }
    }
    _free_task_memory(curtask);
    // 重新创建用户态堆栈
    uint32_t ustack = alloc_pypage();
    map_vm_page(0xFFFF0000, ustack);
    irqframe->if_ESP = 0xFFFF0000 + PAGE_SIZE;
    // 将参数拷贝到用户态堆栈中
    if (argc > 0) {
        irqframe->if_ESP -= argsz*sizeof(uint32_t);
        uint32_t *destargs = (uint32_t *)(irqframe->if_ESP);
        int argi = 0;
        int argoffset = argc;
        int nextparam = 0;
        for (; argi < argc; ++argi) {
            int numlen = param_buf[nextparam];
            uint32_t *srcargs = (uint32_t *)&param_buf[nextparam+1];
            memcpy(&destargs[argoffset], srcargs, numlen*sizeof(uint32_t));
            destargs[argi] = (uint32_t)&destargs[argoffset];
            argoffset += numlen;
            nextparam += numlen + 1;
        }
        irqframe->if_ESP -= 3*sizeof(uint32_t);
        destargs = (uint32_t *)(irqframe->if_ESP);
        destargs[0] = 0;        // _start的返回地址
        destargs[1] = argc;     // 参数个数
        destargs[2] = (uint32_t)&destargs[3]; // 参数
    }
    else {
        irqframe->if_ESP -= 3*sizeof(uint32_t);
        uint32_t *destargs = (uint32_t *)(irqframe->if_ESP);
        destargs[0] = 0;        // _start的返回地址
        destargs[1] = 0;        // 参数个数
        destargs[2] = NULL;     // 参数
    }

    uint32_t sizecnt = 0;
    // 读取elfheader
    uint32_t pyheader = alloc_pypage();
    void *headbuf = (void *)(ELF_FILE);
    map_vm_page(ELF_FILE, pyheader);
    for (int n = 0; n < PAGE_SIZE / BLOCK_SIZE; ++n) {
        file_read(fnode, n * BLOCK_SIZE , headbuf + n * BLOCK_SIZE, BLOCK_SIZE);
    }

    ElfHeader *elfheader = (ElfHeader *)(ELF_FILE);
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
            file_read(  fnode,
                        sizecnt + n * BLOCK_SIZE,
                        buf + n * BLOCK_SIZE,
                        BLOCK_SIZE);
        }
        sizecnt += PAGE_SIZE;
    }

    load_cr3(pdt);
    file_close(fnode);

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
                if (pet[npte] & PAGE_PRESENT) {
                    release_pypage(PAGE_FLOOR(pet[npte]));
                    pet[npte] = 0;  // NOTE:回收页表项
                }
            }
            // NOTE:回收页表项及页表自身(与delete_task配合)
            release_vm_page(pet);
            cur_pdt[npde] = 0;
        }
    }
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
    if (cur_task->ts_wait != NULL)
        wakeup(cur_task->ts_wait);

    switch_task();
    return 0;
}

int
sys_waitpid(IrqFrame *irq, pid_t pid, int *status, int options)
{
    if (pid > 0) {
        Task *task = get_task(pid);
        if (task == NULL)   return -1;
        Task *curtask = current_task();
        if (task->ts_parent != curtask) return -1;

        if (task->ts_state != TS_ZOMBIE) {
            task->ts_wait = curtask;
            sleep(task->ts_wait);
        }
        if (status != NULL)
            *status = task->ts_exit;
        delete_task(task);
        return pid;
    }
    else if (pid < -1) {
        // TODO:wait groupid == |pid|
    }
    else if (pid == 0) {
        // TODO:wait groupid == cur.groupid
    }
    else if (pid == -1) {
        // TODO:wait any
    }

    return 0;
}

int
sys_pause(IrqFrame *irq)
{
    switch_task();
    return 0;
}

int
sys_getpid(IrqFrame *irq)
{
    Task *task = current_task();
    return task->ts_pid;
}
