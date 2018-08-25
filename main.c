#include "sys/types.h"
#include "arch/arch.h"
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

void
file_tail(uint16_t dev, uint16_t inode_num)
{
    uint32_t blk = 0;
    IndexNode *one_inode = get_inode(dev, inode_num);
    if (S_ISREG(one_inode->in_inode.in_file_mode)) {
        uint32_t bytes = one_inode->in_inode.in_file_size - 3;
        uint32_t offset = bytes & (BLOCK_SIZE - 1);
        blk = get_zone(one_inode, bytes);
        printx(blk);
        BlockBuffer *one_buf = get_block(one_inode->in_dev, blk);
        char *content = (char *)one_buf->bf_data + offset;
        print(content);
        release_block(one_buf);
    }
    release_inode(one_inode);
}

void
init_filesystem(uint16_t dev)
{
    init_partion(dev);
    init_super_block(dev);
    dump_super_block(dev);
    init_inodes(dev);
    init_zones(dev);
    // IndexNode* inode_1m = name_to_inode("/1M");
    // ino_t ino_1m = inode_1m->in_inum;
    // release_inode(inode_1m);
    // file_tail(dev, ino_1m);
    IndexNode *node_2m = file_create("/2M", 0, 0);
    // file_create("/2M", 0, 0);
    file_create("/home/3M", 0, 0);
    file_create("/home/3M/4M", 0, 0);
    sync_inodes(dev);

    char bufdata[1024];
    for (int i = 0; i < 1023; ++i)
        bufdata[i] = 'a';
    bufdata[1022] = 'b';
    for (int ii = 0; ii < 70; ++ii) {
        for (int i = 0; i < 1024; ++i) {
            file_write(node_2m, ii * 1024 * 1023 + i*1023, bufdata, 1023);
        }
    }
    release_inode(node_2m);

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
        print(dir.dr_name);
        print(" ");

        // file_tail(dev, dir.dr_inode);

        print("\n");
        file_seek += sizeof(Direction);
    }
    release_block(buf);
}

void
start_main()
{
    init_memory(5*1024*1024, 8*1024*1024);
    init_isa();
    init_disk();
    init_block_buffer();
    init_filesystem(ROOT_DEVICE);
    // init_filesystem();
    // inode_ls(2);
}