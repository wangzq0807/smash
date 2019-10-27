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
copy_args(int frameEsp, int argc, int argsz)
{
    if (argc > 0) {
        frameEsp -= argsz*sizeof(uint32_t);
        uint32_t *destargs = (uint32_t *)(frameEsp);
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
        frameEsp -= 3*sizeof(uint32_t);
        destargs = (uint32_t *)(frameEsp);
        destargs[0] = 0;        // _start的返回地址
        destargs[1] = argc;     // 参数个数
        destargs[2] = (uint32_t)&destargs[3]; // 参数
    }
    else {
        frameEsp -= 3*sizeof(uint32_t);
        uint32_t *destargs = (uint32_t *)(frameEsp);
        destargs[0] = 0;        // _start的返回地址
        destargs[1] = 0;        // 参数个数
        destargs[2] = NULL;     // 参数
    }
    return frameEsp;
}

int
load_elf(IndexNode *fnode)
{
    /*
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
    int frameIp = elfheader->eh_entry;
    uint32_t linear = PAGE_FLOOR(elfheader->eh_entry);
    map_vm_page(linear, pyheader);  // TODO:假设entry和elfheader在同一个页面
    if (linear != ELF_FILE) {
        release_vm_page(headbuf);
        // NOTE: 此时elfheader的指针将失效
        elfheader = NULL;
    }
    sizecnt = PAGE_SIZE;
    // 读取剩余文件内容
    pde_t *pdt = (pde_t *)get_pdt();
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

    load_pdt(pdt);
    return frameIp;*/
    return -1;
}

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

    // 释放当前进程的所有打开文件和物理内存
    Task *curtask = current_task();
    // for (int i = 2; i < MAX_FD; ++i) {
    //     VFile *vf = curtask->ts_filps[i];
    //     if (vf != NULL) {
    //         // file_close(vf->f_inode);
    //         release_vfile(vf);
    //         curtask->ts_filps[i] = NULL;
    //     }
    // }
    VFile* vf = alloc_vfile();
    vf->f_inode = fnode;
    int fd = map_vfile(vf);
    _free_task_memory(curtask);
    // 重新创建用户态堆栈
    uint32_t ustack = alloc_pypage();
    map_vm_page(0xFFFF0000, ustack);
    irqframe->if_ESP = 0xFFFF0000 + PAGE_SIZE;
    // 将参数拷贝到用户态堆栈中
    irqframe->if_ESP = copy_args(irqframe->if_ESP, argc, argsz);

    irqframe->if_EIP = LoadElf(fd, fnode);
    file_close(fnode);

    return 0;
}

static void
_free_task_memory(Task *task)
{
    pdt_t cur_pdt = (pdt_t)PAGE_FLOOR(task->ts_tss.t_CR3);
    for (uint32_t npde = 1; npde < PAGE_INT_SIZE; ++npde) {
        if (cur_pdt[npde] & PAGE_PRESENT) {
            pt_t pt = pde2pt(cur_pdt[npde]);
            for (uint32_t npte = 0; npte < PAGE_INT_SIZE; ++npte) {
                if (pt[npte] & PAGE_PRESENT) {
                    release_pypage(pte2pypage(pt[npte]));
                    pt[npte] = 0;  // NOTE:回收页表项
                }
            }
            // NOTE:回收页表项及页表自身(与delete_task配合)
            release_vm_page((vm_t)pt);
            cur_pdt[npde] = 0;
        }
    }
    load_pdt(cur_pdt);
}

extern int sys_close(IrqFrame *irq, int fd);

static void
_close_task_files(Task *task)
{
    for (int i = 0; i < MAX_FD; ++i) {
        sys_close(NULL, i);
    }
}

int
sys_exit(IrqFrame *irq, int code)
{
    Task *cur_task = current_task();
    cur_task->ts_exit = code;
    cur_task->ts_state = TS_ZOMBIE;
    _close_task_files(cur_task);
    _free_task_memory(cur_task);
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
