#include "arch/irq.h"
#include "arch/task.h"
#include "log.h"
#include "fs/file.h"
#include "fs/vfile.h"
#include "sys/types.h"
#include "sys/fcntl.h"
#include "asm.h"

int
sys_open(IrqFrame *irq, const char *filename, int flags, int mode)
{
    // TODO:权限控制
    IndexNode *fnode = NULL;
    if (mode & O_CREAT)
        fnode = file_create(filename, flags, mode);
    else
        fnode = file_open(filename, flags, mode);
    if (fnode == NULL)  return -1;

    VFile* vfile = alloc_vfile();
    if (vfile == NULL)  return -1;
    vfile->f_mode = mode;
    vfile->f_inode = fnode;
    if (mode & O_TRUNC) {
        file_trunc(fnode);
    }
    if (mode & O_APPEND) {
        vfile->f_seek = fnode->in_inode.in_file_size;
    }

    Task *cur = current_task();
    if (cur->ts_findex < MAX_FD) {
        cur->ts_filps[cur->ts_findex++] = vfile;
        return 0;
    }
    else {
        return -1;
    }
}

int
sys_create(IrqFrame *irq, const char *filename, int flags, int mode)
{
    return sys_open(irq, filename, O_CREAT|O_TRUNC, mode);
}

int
sys_read(IrqFrame *irq, int fd, void *buf, size_t count)
{
    // TODO:权限控制
    VFile* vfile = NULL;
    if (fd < MAX_FD) {
        Task *cur = current_task();
        vfile = cur->ts_filps[fd];
    }
    if (vfile == NULL)  return -1;

    int readcnt = 0;
    while (readcnt < count) {
        int siz = file_read(vfile->f_inode, vfile->f_seek, buf, count);
        if (siz == 0)
            break;
        readcnt += siz;
        vfile->f_seek += siz;
    }

    return 0;
}

int
sys_write(IrqFrame *irq, int fd, const void *buf, size_t count)
{
    // TODO:权限控制
    VFile* vfile = NULL;
    if (fd < MAX_FD) {
        Task *cur = current_task();
        vfile = cur->ts_filps[fd];
    }
    if (vfile == NULL)  return -1;

    int readcnt = 0;
    while (readcnt < count) {
        int siz = file_write(vfile->f_inode, vfile->f_seek, buf, count);
        if (siz == 0)
            break;
        readcnt += siz;
        vfile->f_seek += siz;
    }

    return 0;
}

int
sys_close(IrqFrame *irq, int fd)
{
    VFile* vfile = NULL;
    if (fd < MAX_FD) {
        Task *cur = current_task();
        vfile = cur->ts_filps[fd];
        if (vfile == NULL)  return -1;

        release_inode(vfile->f_inode);
        cur->ts_filps[fd] = NULL;
        release_vfile(vfile);
    }

    return 0;
}