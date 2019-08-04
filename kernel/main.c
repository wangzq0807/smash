#include "sys/types.h"
#include "arch/arch.h"
#include "dev/char/keyboard.h"
#include "log.h"
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
#include "dev/char/serial.h"

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

void
start_main()
{
    init_serial(COM_PORT1);
    init_memory(4*1024*1024, 8*1024*1024);
    init_isa();
    init_keyboard();
    init_filesystem(ROOT_DEVICE);
    start_task();
    // init_filesystem();
    // inode_ls(2);
}