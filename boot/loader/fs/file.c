#include "sys/types.h"
#include "superblk.h"
#include "nodes.h"
#include "sys/stat.h"
#include "string.h"
#include "log.h"
#include "file.h"
#include "disk_drv.h"
#include "defs.h"

IndexNode *
name_to_dirinode(const char *pathname, const char **basename);

IndexNode *
name_to_inode(const char *pathname);

ino_t
search_file(IndexNode *dirinode, const char *fname, int len);

IndexNode *
file_open(const char *pathname, int flags, int mode)
{
    IndexNode *inode = name_to_inode(pathname);
    if (inode == NULL)
        return NULL;

    return inode;
};

ssize_t
file_read(const IndexNode *inode, off_t seek, void *buf, size_t count)
{
    ssize_t ret = 0;
    if (inode->in_file_size < seek)
        return ret;
    count = MIN(inode->in_file_size - seek + 1, count);
    if (count > 0) {
        blk_t blk = get_zone(inode, seek);
        void *blockbuf = ata_read(blk);
        uint32_t offset = seek & (BLOCK_SIZE - 1);
        uint32_t len = MIN(BLOCK_SIZE - offset, count);
        memcpy(buf, blockbuf + offset, len);
        ret += len;
    }
    if (ret < count) {
        uint32_t len = count - ret;
        int blknum = (len + BLOCK_SIZE - 1) >> BLOCK_SHIFT;
        for (int i = 0; i < blknum; ++i)
        {
            blk_t blk = get_zone(inode, seek + ret);
            void *blockbuf = ata_read(blk);
            memcpy(buf + ret, blockbuf, BLOCK_SIZE);
            ret += BLOCK_SIZE;
        }
    }

    return ret;
}

ssize_t
file_write(IndexNode *inode, off_t seek, const void *buf, size_t count)
{
    return 0;
}

int
file_close(IndexNode *inode)
{
    return 0;
}

uint32_t
_next_file(IndexNode *inode, uint32_t next, Direction *dir)
{
    // TODO: 性能待优化
    uint32_t offset = next & (BLOCK_SIZE - 1);
    uint32_t zone = get_zone(inode, next);
    // TODO: zone和block
    void *blockbuf = ata_read(zone);
    memcpy(dir, blockbuf + offset, sizeof(Direction));

    return next + sizeof(Direction);
}

ino_t
search_file(IndexNode *inode, const char *fname, int len)
{
    if (!S_ISDIR(inode->in_file_mode))    return INVALID_INODE;

    off_t seek = 0;
    while (seek < inode->in_file_size) {
        Direction dir;
        seek = _next_file(inode, seek, &dir);
        if (dir.dr_inode != INVALID_INODE &&
            dir.dr_name[len] == 0 &&
            strncmp(fname, dir.dr_name, len) == 0) {
            return dir.dr_inode;
        }
    }
    return INVALID_INODE;
}

IndexNode *
name_to_dirinode(const char *pathname, const char **basename)
{
    ino_t work_inode = ROOT_INODE;
    if (pathname[0] == '/') {
        work_inode = ROOT_INODE;
        pathname += 1;
    }
    else {
        return NULL;
    }

    while (*pathname) {
        const char *next = strstr(pathname, "/");
        const int len = next - pathname;
        if (*next == '/' && *(next+1) != 0) {
            IndexNode *inode = get_inode(work_inode);
            work_inode = search_file(inode, pathname, len);
            if (work_inode == INVALID_INODE) {
                KLOG(ERROR, "name_to_dirinode %s not found", pathname);
                return NULL;
            }
            pathname = next + 1;
        }
        else {
            break;
        }
    }
    if (basename != NULL)
        *basename = pathname;
    return get_inode(work_inode);
}

IndexNode *
name_to_inode(const char *pathname)
{
    const char *basename = NULL;
    IndexNode *ret_inode = NULL;
    IndexNode *dirnode = name_to_dirinode(pathname, &basename);
    if (dirnode == NULL)    return NULL;

    if (basename != NULL && basename[0] != 0) {
        // TODO : 对结尾为'/'的文件路径进行检查
        int len = strlen(basename);
        if (basename[len -1] == '/')
            len -= 1;
        ino_t ino = search_file(dirnode, basename, len);
        if (ino != INVALID_INODE)
            ret_inode = get_inode(ino);
    }
    else {
        ret_inode = dirnode;
    }

    return ret_inode;
}