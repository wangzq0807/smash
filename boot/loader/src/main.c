#include "types.h"
#include "fs/disk_drv.h"
#include "fs/superblk.h"
#include "fs/file.h"
#include "log.h"
#include "elf.h"
#include "memory.h"
#include "asm.h"

#define SMASH_BIN   ("/boot/smash")
int LoadKernel(char *path);

void
start_main()
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

    ljmp(0x8, vaddr);
    return 0;
}
