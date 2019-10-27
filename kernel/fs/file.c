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

IndexNode *
file_open(const char *pathname, int flags, int mode)
{
    IndexNode *inode = name_to_inode(pathname);
    if (inode == NULL)
        return NULL;

    return inode;
};

IndexNode *
file_create(const char *pathname, int flags, int mode)
{
    const char *basename = NULL;
    IndexNode *ret_inode = NULL;
    IndexNode *dinode = name_to_dirinode(pathname, &basename);
    if (dinode == NULL) return NULL;

    ino_t ino = search_file(dinode, basename, FILENAME_LEN);
    if (ino == INVALID_INODE) {
        // create new file
        ret_inode = alloc_inode(dinode->in_dev);
        ret_inode->in_inode.in_file_mode = mode;
        ret_inode->in_inode.in_num_links = 1;
        ret_inode->in_inode.in_file_size = 0;

        add_file_entry(dinode, basename, ret_inode);
    }
    else {
        ret_inode = get_inode(dinode->in_dev, ino);
    }
    sync_dev(dinode->in_dev);
    release_inode(dinode);

    return ret_inode;
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
        uint32_t len = count - ret;
        int blknum = (len + BLOCK_SIZE - 1) >> BLOCK_LOG_SIZE;
        for (int i = 0; i < blknum; ++i)
        {
            blk_t blk = get_zone(inode, seek + ret);
            BlockBuffer *blkbuf = get_block(inode->in_dev, blk);
            
            memcpy(buf + ret, blkbuf->bf_data, BLOCK_SIZE);
            release_block(blkbuf);
            ret += BLOCK_SIZE;
        }
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
    sync_dev(inode->in_dev);
    return ret;
}

int
file_close(IndexNode *inode)
{
    if (inode == NULL)  return 0;
    release_inode(inode);
    return 0;
}

int
file_trunc(IndexNode *inode)
{
    int ret = truncate_zones(inode);
    sync_dev(inode->in_dev);
    return ret;
}

int
file_link(const char *pathname, IndexNode *inode)
{
    if (!S_ISREG(inode->in_inode.in_file_mode))  return -1;

    int ret = 0;
    const char *basename = NULL;
    IndexNode *dinode = name_to_dirinode(pathname, &basename);
    if (dinode == NULL) return NULL;

    ino_t ino = search_file(dinode, basename, FILENAME_LEN);
    if (ino == INVALID_INODE) {
        add_file_entry(dinode, basename, inode);
    }
    else {
        ret = -1;
    }
    release_inode(dinode);
    return ret;
}

int
file_unlink(const char *pathname)
{
    int ret = 0;
    const char *basename = NULL;
    IndexNode *dinode = name_to_dirinode(pathname, &basename);
    if (dinode == NULL) return NULL;

    ret = rm_file_entry(dinode, basename);

    release_inode(dinode);
    return ret;
}
