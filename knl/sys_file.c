#include "arch/irq.h"
#include "arch/task.h"
#include "log.h"
#include "fs/file.h"
#include "fs/vfile.h"
#include "fs/path.h"
#include "sys/types.h"
#include "sys/fcntl.h"
#include "sys/stat.h"
#include "asm.h"
#include "dev/char/tty.h"

int
sys_open(IrqFrame *irq, const char *filename, int flags, int mode)
{
    // TODO:权限控制
    IndexNode *fnode = NULL;
    if (flags & O_CREAT)
        fnode = file_create(filename, flags, mode);
    else
        fnode = file_open(filename, flags, mode);
    if (fnode == NULL)  return -1;

    VFile* vfile = alloc_vfile();
    if (vfile == NULL)  return -1;
    vfile->f_mode = mode;
    vfile->f_inode = fnode;
    if (flags & O_TRUNC) {
        file_trunc(fnode);
    }
    if (flags & O_APPEND) {
        vfile->f_seek = fnode->in_inode.in_file_size;
    }

    Task *cur = current_task();
    if (cur->ts_findex < MAX_FD) {
        cur->ts_filps[cur->ts_findex] = vfile;
        return cur->ts_findex;
    }
    else {
        return -1;
    }
}

int
sys_creat(IrqFrame *irq, const char *filename, int flags, int mode)
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
    if (S_ISCHR(vfile->f_inode->in_inode.in_file_mode)) {
        return tty_read(buf, count);
    }
    while (readcnt < count) {
        int siz = file_read(vfile->f_inode, vfile->f_seek, buf, count);
        if (siz == 0)
            break;
        readcnt += siz;
        vfile->f_seek += siz;
    }

    return readcnt;
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

    int writecnt = 0;
    if (S_ISCHR(vfile->f_inode->in_inode.in_file_mode)) {
        return tty_write(buf, count);
    }
    while (writecnt < count) {
        int siz = file_write(vfile->f_inode, vfile->f_seek, buf, count);
        if (siz == 0)
            break;
        writecnt += siz;
        vfile->f_seek += siz;
    }

    return writecnt;
}

int
sys_close(IrqFrame *irq, int fd)
{
    VFile* vfile = NULL;
    if (fd < MAX_FD) {
        Task *cur = current_task();
        vfile = cur->ts_filps[fd];
        if (vfile == NULL)  return -1;

        file_close(vfile->f_inode);
        cur->ts_filps[fd] = NULL;
        release_vfile(vfile);
    }

    return 0;
}

int
sys_link(IrqFrame *irq, const char *oldpath, const char *newpath)
{
    IndexNode *oldnode = file_open(oldpath, O_RDONLY, 0);
    if (oldnode == NULL)    return -1;

    int ret = file_link(newpath, oldnode);

    file_close(oldnode);

    return ret;
}

int
sys_unlink(IrqFrame *irq, const char *pathname)
{
    IndexNode *fnode = file_open(pathname, O_RDONLY, 0);
    if (fnode == NULL)  return -1;

    int ret = file_unlink(pathname, fnode);

    file_close(fnode);
    return ret;
}

int
sys_mkdir(IrqFrame *irq, const char *pathname, int mode)
{
    make_dir(pathname, mode);
    return 0;
}

int
sys_rmdir(IrqFrame *irq, const char *pathname)
{
    rm_dir(pathname);
    return 0;
}

int
sys_chdir(IrqFrame *irq, const char *path)
{
    return 0;
}

int
sys_mknod(IrqFrame *irq, const char *path, mode_t mode, dev_t dev)
{
    return 0;
}

int
sys_chmod(IrqFrame *irq, const char *path, mode_t mode)
{
    IndexNode *finode = name_to_inode(path);
    if (finode == NULL)  return -1;

    finode->in_inode.in_file_mode = mode;
    finode->in_status |= INODE_DIRTY;
    release_inode(finode);

    return 0;
}
