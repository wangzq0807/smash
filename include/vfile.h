#ifndef __VFILE_H__
#define __VFILE_H__
#include "sys/types.h"
#include "list.h"

typedef struct _VFile VFile;
struct _VFile {
    dev_t               f_dev;
    ino_t               f_inode;
    int                 f_refs;
    int                 f_mode;
    off_t               f_seek;
    ListEntity          f_link;
};

void
init_vfiles();

VFile *
add_vfile_refs(VFile *file);

void
release_vfile(VFile *file);

VFile *
vfile_create(const char *pathname, int flags, int mode);

VFile *
vfile_open(const char *pathname, int flags, int mode);

ssize_t
vfile_read(const char *pathname, void *buf, size_t count);

ssize_t
vfile_write(const char *pathname, const void *buf, size_t count);

int
vfile_close(VFile *file);

#endif // __VFILE_H__
