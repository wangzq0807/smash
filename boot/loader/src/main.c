#include "types.h"
#include "fs/disk_drv.h"
#include "fs/superblk.h"
#include "fs/file.h"
#include "log.h"
#include "elf.h"
#include "memory.h"

#define SMASH_BIN   ("/boot/smash")
int LoadKernel(char *path);

void
start_main()
{
    init_memory();
    init_file_system();
    LoadKernel(SMASH_BIN);
}

int
LoadKernel(char *path)
{
    IndexNode *fnode = file_open(path, 0, 0);
    LoadElf(fnode);
    return 0;
}
