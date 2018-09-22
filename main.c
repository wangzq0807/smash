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

void
init_filesystem(uint16_t dev)
{
    init_partion(dev);
    init_super_block(dev);
    dump_super_block(dev);
    init_inodes(dev);
    init_zones(dev);
    init_vfiles();

    IndexNode *inode = get_inode(dev, ROOT_INODE);
    uint32_t blk = 0;
    blk = get_zone(inode, 0);

    BlockBuffer *buf = get_block(inode->in_dev, blk);
    uint8_t *data = buf->bf_data;
    uint32_t file_size = inode->in_inode.in_file_size;
    release_inode(inode);

    Direction dir;
    uint32_t file_seek = 0;
    while (file_seek < file_size) {
        memcpy(&dir, data+file_seek, sizeof(Direction));
        printk(dir.dr_name);
        printk(" ");

        // file_tail(dev, dir.dr_inode);

        printk("\n");
        file_seek += sizeof(Direction);
    }
    release_block(buf);
}

void
start_main()
{
    init_memory(4*1024*1024, 8*1024*1024);
    init_isa();
    init_keyboard();
    init_disk();
    init_block_buffer();
    init_filesystem(ROOT_DEVICE);
    start_task();
    // init_filesystem();
    // inode_ls(2);
}