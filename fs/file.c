#include "defs.h"
#include "path.h"
#include "fsdefs.h"
#include "inodes.h"
#include "zones.h"
#include "sys/stat.h"
#include "string.h"
#include "buffer.h"
#include "log.h"

 int
_file_append(struct IndexNode *inode, void *data, int len)
{
    if (len > BLOCK_SIZE )
        return -1;
    file_offset_t tail = inode->in_inode.in_file_size;
    blk_t blk = get_zone(inode, tail);
    int offset = tail % BLOCK_SIZE;
    int less = BLOCK_SIZE - offset;
    int more = len - less;

    struct BlockBuffer *buf = get_block(inode->in_dev, blk);
    memcpy(buf->bf_data + offset, data, MIN(len, less));
    release_block(buf);

    if (more > 0) {
        blk_t new_blk = alloc_zone(inode);
        struct BlockBuffer *new_buf = get_block(inode->in_dev, new_blk);
        memcpy(new_buf->bf_data, data+less, more);
        release_block(new_buf);
    }
    inode->in_inode.in_file_size += len;

    return 0;
}

int
file_open(const char *pathname, int flags)
{
    struct IndexNode *drinode = dirname_to_inode(pathname);
    if (drinode == NULL) 
        return -1;

    return 0;
};

int
file_create(const char *pathname, int mode)
{
    struct IndexNode *drinode = dirname_to_inode(pathname);
    if (drinode == NULL) 
        return -1;
    const char *fname = file_name(pathname);
    struct Direction dir;
    memcpy(dir.dr_name, fname, strlen(fname)+1);

    struct IndexNode *inode = alloc_inode(1);
    inode->in_inode.in_file_mode = S_IFREG | S_IRUSR | S_IWUSR;
    inode->in_inode.in_num_links = 1;

    dir.dr_inode = inode->in_inum;
    _file_append(drinode, &dir, sizeof(struct Direction));
    release_inode(drinode);
    
    return 0;
}
