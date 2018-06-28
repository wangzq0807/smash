#include "defs.h"
#include "hard_disk.h"
#include "partion.h"
#include "superblk.h"
#include "inodes.h"
#include "arch/arch.h"
#include "log.h"
#include "string.h"
#include "buffer.h"
#include "memory.h"
#include "fs.h"
#include "sys/stat.h"
#include "zones.h"
#include "path.h"

void
file_tail(uint16_t dev, uint16_t inode_num)
{
    uint32_t blk = 0;
    uint32_t offset = 0;
    struct IndexNode *one_inode = get_inode(dev, inode_num);
    if (S_ISREG(one_inode->in_inode.in_file_mode)) {
        uint32_t bytes = one_inode->in_inode.in_file_size - 3;
        blk = get_zone(one_inode, bytes, &offset);
        printx(blk);
        struct BlockBuffer *one_buf = get_block(one_inode->in_dev, blk);
        char *content = (char *)one_buf->bf_data + offset;
        print(content);
        release_block(one_buf);
    }
    release_inode(one_inode);
}

static void
init_filesystem(uint16_t dev)
{
    init_partion(dev);
    init_super_block(dev);
    dump_super_block(dev);
    init_inodes(dev);
    init_zones(dev);
    uint32_t node_66m = name_to_inode("./home/cdos/../cdos/66M");
    file_tail(dev, node_66m);

    struct IndexNode *inode = get_inode(dev, 1);
    uint32_t blk = 0;
    uint32_t offset = 0;
    blk = get_zone(inode, 0, &offset);

    struct BlockBuffer *buf = get_block(inode->in_dev, blk);
    uint8_t *data = buf->bf_data;
    uint32_t file_size = inode->in_inode.in_file_size;
    release_inode(inode);

    struct Direction dir;
    uint32_t file_seek = 0;
    while (file_seek < file_size) {
        memcpy(&dir, data+file_seek, sizeof(struct Direction));
        print(dir.dr_name);
        print(" ");

        file_tail(dev, dir.dr_inode);

        print("\n");
        file_seek += sizeof(struct Direction);
    }
    release_block(buf);
}

void
start_main()
{
    init_cpu();
    init_memory(5*1024*1024, 64*1024*1024);
    init_disk();
    init_block_buffer();
    init_filesystem(ROOT_DEVICE);
    // init_filesystem();
    // inode_ls(2);
}