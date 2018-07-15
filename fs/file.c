#include "sys/types.h"
#include "path.h"
#include "fsdefs.h"
#include "inodes.h"
#include "zones.h"
#include "sys/stat.h"
#include "string.h"
#include "buffer.h"
#include "log.h"

static int
_file_append(IndexNode *inode, void *data, int len)
{
    if (len > BLOCK_SIZE )
        return -1;
    file_offset_t tail = inode->in_inode.in_file_size;
    blk_t blk = get_zone(inode, tail);
    int offset = tail % BLOCK_SIZE;
    int less = BLOCK_SIZE - offset;
    int more = len - less;

    BlockBuffer *buf = get_block(inode->in_dev, blk);
    memcpy(buf->bf_data + offset, data, MIN(len, less));
    buf->bf_status |= BUF_DIRTY;
    release_block(buf);

    if (more > 0) {
        blk_t new_blk = alloc_zone(inode);
        BlockBuffer *new_buf = get_block(inode->in_dev, new_blk);
        memcpy(new_buf->bf_data, data+less, more);
        buf->bf_status |= BUF_DIRTY;
        release_block(new_buf);
    }
    inode->in_inode.in_file_size += len;

    return 0;
}

int
file_open(const char *pathname, int flags, int mode)
{
    const char *remain = NULL;
    IndexNode *inode = name_to_inode(pathname, &remain);
    if (*remain != 0 || inode == NULL) 
        return -1;

    return 0;
};

int
file_create(const char *pathname, int flags, int mode)
{
    const char *remain = NULL;
    IndexNode *inode = name_to_inode(pathname, &remain);
    if (*remain != 0) {
        const char *subdir = strstr(remain, "/");
        if (*subdir == 0) {
            Direction dir;
            memcpy(dir.dr_name, remain, strlen(remain)+1);

            IndexNode *new_inode = alloc_inode(ROOT_DEVICE);
            new_inode->in_inode.in_file_mode = S_IFREG | S_IRUSR | S_IWUSR;
            new_inode->in_inode.in_num_links = 1;
            dir.dr_inode = new_inode->in_inum;
            release_inode(new_inode);

            _file_append(inode, &dir, sizeof(Direction));
            inode->in_status |= INODE_DIRTY;
        }
    }
    else {

    }
    release_inode(inode);

    return 0;
}

ssize_t
file_read(const char *pathname, void *buf, size_t count)
{
    return 0;
}

ssize_t
file_write(const char *pathname, const void *buf, size_t count)
{
    return 0;
}
