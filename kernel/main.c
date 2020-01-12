#include "sys/types.h"
#include "arch/arch.h"
#include "dev/char/keyboard.h"
#include "lib/log.h"
#include "string.h"
#include "memory.h"
#include "sys/stat.h"
#include "fs/fsdefs.h"
#include "fs/hard_disk.h"
#include "fs/buffer.h"
#include "fs/partion.h"
#include "fs/superblk.h"
#include "fs/inodes.h"
#include "fs/zones.h"
#include "fs/path.h"
#include "fs/file.h"
#include "arch/task.h"

void
init_filesystem(uint16_t dev);

void
start_main()
{
    // 设备初始化
    init_isa();
    KLOG(DEBUG, "start main!\n");
    memory_setup();
    init_keyboard();
    init_filesystem(ROOT_DEVICE);
    start_task();
    // init_filesystem();
    // inode_ls(2);
}

void
init_filesystem(uint16_t dev)
{
    init_disk();
    init_block_buffer();
    init_partion(dev);
    init_super_block(dev);
    dump_super_block(dev);
    init_inodes(dev);
    init_zones(dev);
    init_vfiles();
}
