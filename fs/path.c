#include "path.h"
#include "string.h"
#include "inodes.h"
#include "zones.h"
#include "sys/stat.h"
#include "buffer.h"
#include "log.h"
#include "asm.h"
#include "file.h"
#include "asm.h"
#include "arch/task.h"

uint16_t cur_inode = ROOT_INODE;
uint16_t cur_dev = ROOT_DEVICE;

uint32_t
_next_file(IndexNode *inode, uint32_t next, Direction *dir)
{
    // TODO: 性能待优化
    uint32_t offset = next & (BLOCK_SIZE - 1);
    uint32_t zone = get_zone(inode, next);
    // TODO: zone和block
    BlockBuffer *blk = get_block(inode->in_dev, zone);
    memcpy(dir, blk->bf_data + offset, sizeof(Direction));
    release_block(blk);

    return next + sizeof(Direction);
}

ino_t
search_file(IndexNode *inode, const char *fname, int len)
{
    if (!S_ISDIR(inode->in_inode.in_file_mode))    return -1;

    off_t seek = 0;
    while (seek < inode->in_inode.in_file_size) {
        Direction dir;
        seek = _next_file(inode, seek, &dir);
        if (dir.dr_inode != INVALID_INODE &&
            strncmp(fname, dir.dr_name, len) == 0 ) {
            return dir.dr_inode;
        }
    }
    return INVALID_INODE;
}

IndexNode *
name_to_dirinode(const char *pathname, const char **basename)
{
    ino_t work_inode = ROOT_INODE;
    dev_t work_dev = ROOT_DEVICE;
    if (pathname[0] == '/') {
        work_inode = ROOT_INODE;
        work_dev = ROOT_DEVICE;
        pathname += 1;
    }
    else {
        Task *curtask = current_task();
        work_inode = curtask->ts_cinode;
        work_dev = curtask->ts_cdev;
    }

    while (*pathname) {
        const char *next = strstr(pathname, "/");
        const int len = next - pathname;
        if (*next == '/' && *(next+1) != 0) {
            IndexNode *inode = get_inode(work_dev, work_inode);
            work_inode = search_file(inode, pathname, len);
            // TODO: 不确定
            work_dev = inode->in_dev;
            release_inode(inode);
            pathname = next + 1;
        }
        else {
            break;
        }
    }
    if (basename != NULL)
        *basename = pathname;
    return get_inode(work_dev, work_inode);
}

IndexNode *
name_to_inode(const char *pathname)
{
    const char *basename = NULL;
    IndexNode *ret_inode = NULL;
    IndexNode *dirnode = name_to_dirinode(pathname, &basename);
    if (dirnode == NULL)    return NULL;

    ino_t ino = search_file(dirnode, basename, FILENAME_LEN);
    if (ino != INVALID_INODE)
        ret_inode = get_inode(dirnode->in_dev, ino);

    release_inode(dirnode);
    return ret_inode;
}

int
add_file_entry(IndexNode *dinode, const char *fname, IndexNode *inode)
{
    if (!S_ISDIR(dinode->in_inode.in_file_mode))    return NULL;

    Direction dir;
    strcpy(dir.dr_name, fname);
    dir.dr_inode = inode->in_inum;

    off_t seek = 0;
    while (seek < dinode->in_inode.in_file_size) {
        Direction dir;
        seek = _next_file(dinode, seek, &dir);
        if (dir.dr_inode == INVALID_INODE) {
            seek -= sizeof(Direction);
            file_write(dinode, seek, (void *)&dir, sizeof(Direction));
        }
    }
    file_write(dinode, dinode->in_inode.in_file_size, (void *)&dir, sizeof(Direction));
    // dinode->in_inode.in_num_links++;
    inode->in_inode.in_num_links++;
    return 0;
}

int
rm_file_entry(IndexNode *dinode, const char *fname)
{
    if (!S_ISDIR(dinode->in_inode.in_file_mode))    return -1;

    off_t seek = 0;
    while (seek < dinode->in_inode.in_file_size) {
        Direction dir;
        seek = _next_file(dinode, seek, &dir);
        if (dir.dr_inode != INVALID_INODE &&
            strncmp(fname, dir.dr_name, FILENAME_LEN) == 0 ) {

            IndexNode *subinode = get_inode(dinode->in_dev, dir.dr_inode);
            subinode->in_inode.in_num_links--;
            subinode->in_status |= INODE_DIRTY;
            if (subinode->in_inode.in_num_links == 0) {
                file_trunc(subinode);
                delete_inode(subinode);
            }
            else {
                release_inode(subinode);
            }

            dir.dr_inode = INVALID_INODE;
            seek -= sizeof(Direction);
            file_write(dinode, seek, (void *)&dir, sizeof(Direction));
            // dinode->in_inode.in_num_links--;
            sync_dev(dinode->in_dev);
            break;
        }
    }
    return 0;
}

int
change_dir(const char *pathname)
{
    const char *basename = NULL;
    IndexNode *dirnode = name_to_dirinode(pathname, &basename);
    if (dirnode == NULL)    return -1;

    IndexNode *inode = NULL;
    ino_t ino = search_file(dirnode, basename, FILENAME_LEN);
    if (ino != INVALID_INODE)
        inode = get_inode(dirnode->in_dev, ino);
    if (inode == NULL) {
        release_inode(dirnode);
        release_inode(inode);
        return -1;
    }

    Task *curtask = current_task();
    if (S_ISDIR(inode->in_inode.in_file_mode)) {
        curtask->ts_cdev = inode->in_dev;
        curtask->ts_cinode = inode->in_inum;
        release_inode(inode);
        release_inode(dirnode);
    }

    return -1;
}

int
make_dir(const char *pathname, int mode)
{
    const char *basename = NULL;
    IndexNode *dirnode = name_to_dirinode(pathname, &basename);
    if (dirnode == NULL)    return -1;

    ino_t ino = search_file(dirnode, basename, FILENAME_LEN);
    if (ino != INVALID_INODE)   return -1;

    IndexNode *subinode = alloc_inode(dirnode->in_dev);
    subinode->in_inode.in_file_mode = mode | S_IFDIR;
    add_file_entry(dirnode, basename, subinode);
    add_file_entry(subinode, ".", subinode);
    add_file_entry(subinode, "..", dirnode);
    release_inode(subinode);
    sync_dev(dirnode->in_dev);
    release_inode(dirnode);

    return 0;
}

int
rm_dir(const char *pathname)
{
    const char *basename = NULL;
    IndexNode *dirnode = name_to_dirinode(pathname, &basename);
    if (dirnode == NULL)    return -1;

    ino_t ino = search_file(dirnode, basename, FILENAME_LEN);
    if (ino == INVALID_INODE)   return -1;

    IndexNode *subinode = get_inode(dirnode->in_dev, ino);

    off_t seek = 0;
    while (seek < subinode->in_inode.in_file_size) {
        Direction dir;
        seek = _next_file(subinode, seek, &dir);
        if (strcmp(dir.dr_name, ".") != 0 &&
            strcmp(dir.dr_name, "..") != 0 ) {
            return -1;
        }
    }
    rm_file_entry(subinode, ".");
    rm_file_entry(subinode, "..");
    rm_file_entry(dirnode, basename);
    sync_dev(dirnode->in_dev);
    release_inode(subinode);
    release_inode(dirnode);

    return 0;
}
