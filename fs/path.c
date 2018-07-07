#include "path.h"
#include "string.h"
#include "inodes.h"
#include "zones.h"
#include "sys/stat.h"
#include "buffer.h"
#include "log.h"
#include "asm.h"

#define PER_BLOCK_BYTES     (1 << BLOCK_LOG_SIZE)

uint16_t cur_inode = ROOT_INODE;
uint16_t cur_dev = ROOT_DEVICE;

uint32_t
_next_file(struct IndexNode *inode, uint32_t next, struct Direction *dir)
{
    uint32_t offset = next & (PER_BLOCK_BYTES - 1);
    uint32_t zone = get_zone(inode, next);
    // TODO: zone和block
    struct BlockBuffer *blk = get_block(inode->in_dev, zone);
    memcpy(dir, blk->bf_data + offset, sizeof(struct Direction));
    release_block(blk);

    return next + sizeof(struct Direction);
}

static ino_t
_search_file(struct IndexNode *inode, const char *name, int len)
{
    if (S_ISDIR(inode->in_inode.in_file_mode)) {
        seek_t seek = 0;
        while (seek < inode->in_inode.in_file_size) {
            struct Direction dir;
            seek = _next_file(inode, seek, &dir);
            if (strncmp(name, dir.dr_name, len) == 0) {
                return dir.dr_inode;
            }
        }
    }
    return INVALID_INODE;
}

struct IndexNode *
dirname_to_inode(const char *name)
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
        if (*next == 0)
            return get_inode(work_dev, work_inode);

        const int len = next - name;
        struct IndexNode *inode = get_inode(work_dev, work_inode);
        work_inode = _search_file(inode, name, len);
        // TODO: 不确定
        work_dev = inode->in_dev;
        release_inode(inode);

        if (work_inode == INVALID_INODE)
            return NULL;

        name = next + 1;
    }
    return NULL;
}

struct IndexNode *
name_to_inode(const char *name)
{
    struct IndexNode *inode = dirname_to_inode(name);
    if (inode == NULL)
        return NULL;
    const char *fname = file_name(name);
    dev_t work_dev = inode->in_dev;
    ino_t work_inode = _search_file(inode, fname, strlen(fname));
    release_inode(inode);

    if (work_inode == INVALID_INODE)
        return NULL;
    else
        return get_inode(work_dev, work_inode);
}