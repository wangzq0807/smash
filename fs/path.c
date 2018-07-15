#include "path.h"
#include "string.h"
#include "inodes.h"
#include "zones.h"
#include "sys/stat.h"
#include "buffer.h"
#include "log.h"
#include "asm.h"

uint16_t cur_inode = ROOT_INODE;
uint16_t cur_dev = ROOT_DEVICE;

uint32_t
_next_file(IndexNode *inode, uint32_t next, Direction *dir)
{
    uint32_t offset = next & (BLOCK_SIZE - 1);
    uint32_t zone = get_zone(inode, next);
    // TODO: zone和block
    BlockBuffer *blk = get_block(inode->in_dev, zone);
    memcpy(dir, blk->bf_data + offset, sizeof(Direction));
    release_block(blk);

    return next + sizeof(Direction);
}

static ino_t
_search_file(IndexNode *inode, const char *name, int len)
{
    if (S_ISDIR(inode->in_inode.in_file_mode)) {
        off_t seek = 0;
        while (seek < inode->in_inode.in_file_size) {
            Direction dir;
            seek = _next_file(inode, seek, &dir);
            if (strncmp(name, dir.dr_name, len) == 0) {
                return dir.dr_inode;
            }
        }
    }
    return INVALID_INODE;
}

IndexNode *
name_to_inode(const char *name, const char **remain)
{
    ino_t work_inode = ROOT_INODE;
    dev_t work_dev = ROOT_DEVICE;
    if (name[0] == '/') {
        work_inode = ROOT_INODE;
        work_dev = ROOT_DEVICE;
        name += 1;
    }
    else {
        work_inode = cur_inode;
        work_dev = cur_dev;
    }

    while (*name) {
        const char *next = strstr(name, "/");
        const int len = next - name;
        IndexNode *inode = get_inode(work_dev, work_inode);
        ino_t next_inode = _search_file(inode, name, len);
        // TODO: 不确定
        work_dev = inode->in_dev;
        release_inode(inode);

        if (next_inode == INVALID_INODE)
            break;
        if (*next == '/')
            name = next + 1;
        else if (*next == 0)
            name = next;
        work_inode = next_inode;
    }
    if (remain != NULL)
        *remain = name;
    return get_inode(work_dev, work_inode);
}
