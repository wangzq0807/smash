#include "sys/types.h"
#include "path.h"
#include "fsdefs.h"
#include "inodes.h"
#include "zones.h"
#include "sys/stat.h"
#include "string.h"
#include "buffer.h"
#include "log.h"
#include "file.h"

static int
_file_append(IndexNode *inode, void *data, int len)
{
    if (len > BLOCK_SIZE )
        return -1;
    off_t tail = inode->in_inode.in_file_size;
    blk_t blk = get_zone(inode, tail);
    int offset = tail & (BLOCK_SIZE - 1);
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
        new_buf->bf_status |= BUF_DIRTY;
        release_block(new_buf);
    }
    inode->in_inode.in_file_size += len;

    return 0;
}

IndexNode *
file_open(const char *pathname, int flags, int mode)
{
    const char *remain = NULL;
    IndexNode *inode = name_to_inode(pathname, &remain);
    if (*remain != 0 || inode == NULL) 
        return NULL;

    return inode;
};

IndexNode *
file_create(const char *pathname, int flags, int mode)
{
    const char *remain = NULL;
    IndexNode *inode = name_to_inode(pathname, &remain);
    if (*remain != 0) {
        // create new file
        const char *subdir = strstr(remain, "/");
        if (*subdir == 0) {
            Direction dir;
            memcpy(dir.dr_name, remain, strlen(remain)+1);

            IndexNode *new_inode = alloc_inode(inode->in_dev);
            new_inode->in_inode.in_file_mode = S_IFREG | S_IRUSR | S_IWUSR;
            new_inode->in_inode.in_num_links = 1;
            new_inode->in_inode.in_file_size = 0;
            dir.dr_inode = new_inode->in_inum;

            _file_append(inode, &dir, sizeof(Direction));
            inode->in_status |= INODE_DIRTY;
            release_inode(inode);
            return new_inode;
        }
    }
    else {
        // open org file
        return inode;
    }
    return NULL;
}

ssize_t
file_read(const IndexNode *inode, off_t seek, void *buf, size_t count)
{
    ssize_t ret = 0;
    if (inode->in_inode.in_file_size < seek)
        return ret;
    count = MIN(inode->in_inode.in_file_size - seek + 1, count);
    if (count > 0) {
        blk_t blk = get_zone(inode, seek);
        BlockBuffer *blkbuf = get_block(inode->in_dev, blk);
        uint32_t offset = seek & (BLOCK_SIZE - 1);
        uint32_t len = MIN(BLOCK_SIZE - offset, count);
        memcpy(buf, blkbuf->bf_data + offset, len);
        release_block(blkbuf);
        ret += len;
    }
    if (ret < count) {
        blk_t blk = get_zone(inode, seek + ret);
        BlockBuffer *blkbuf = get_block(inode->in_dev, blk);
        uint32_t len = count - ret;
        memcpy(buf + ret, blkbuf->bf_data, len);
        release_block(blkbuf);
        ret += len;
    }

    return ret;
}

ssize_t
file_write(IndexNode *inode, off_t seek, const void *buf, size_t count)
{
    ssize_t ret = 0;
    const blk_t new_blknum = (seek + count + BLOCK_SIZE - 1) >> BLOCK_LOG_SIZE;
    const blk_t cur_blknum = (inode->in_inode.in_file_size + BLOCK_SIZE - 1)>> BLOCK_LOG_SIZE;
    if (new_blknum > cur_blknum) {
        alloc_zone(inode);
    }
    blk_t blk = get_zone(inode, seek);
    BlockBuffer *blkbuf = get_block(inode->in_dev, blk);
    uint32_t offset = seek & (BLOCK_SIZE - 1);
    uint32_t remain = MIN(BLOCK_SIZE - offset, count);
    memcpy(blkbuf->bf_data + offset, buf, remain);
    blkbuf->bf_status |= BUF_DIRTY;
    release_block(blkbuf);
    ret += remain;

    uint32_t more = count - remain;
    if (more > 0) {
        blk_t next_blk = get_zone(inode, seek + ret);
        BlockBuffer *new_buf = get_block(inode->in_dev, next_blk);
        memcpy(new_buf->bf_data, buf + ret, more);
        new_buf->bf_status |= BUF_DIRTY;
        release_block(new_buf);
        ret += more;
    }
    inode->in_inode.in_file_size = MAX(inode->in_inode.in_file_size, seek + ret);
    inode->in_status |= INODE_DIRTY;
    return ret;
}

int
file_close(IndexNode *inode)
{
    return 0;
}

int
file_trunc(IndexNode *inode)
{
    return truncate_zones(inode);
}
