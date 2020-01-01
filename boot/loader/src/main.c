#include "types.h"
#include "fs/disk_drv.h"
#include "fs/superblk.h"
#include "fs/file.h"
#include "log.h"
#include "elf.h"
#include "memory.h"
#include "asm.h"
#include "multiboot.h"

#define SMASH_BIN   ("/boot/smash")
int LoadKernel(char *path);

void
start_bootloader()
{
    init_memory();
    init_file_system();
    LoadKernel(SMASH_BIN);
    while (1)
    {
        smash_memory();
    }
}

int
LoadKernel(char *path)
{
    IndexNode *fnode = file_open(path, 0, 0);
    if (fnode == NULL)
    {
        KLOG(ERROR, "kernel not exist !");
        return -1;
    }
    vm_t vaddr = LoadElf(fnode);

    struct { uint32_t o; uint32_t s; } laddr;
    laddr.o = vaddr;
    laddr.s = 0x8;
    __asm__ volatile (
        "ljmp *%0 \n"
        : :"m"(laddr.o), "m"(laddr.s), "b"(MULTIBOOT_HEADER_MAGIC), "a"(0)
    );
    return 0;
}
