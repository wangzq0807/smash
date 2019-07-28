#ifndef __VFILE_H__
#define __VFILE_H__
#include "sys/types.h"
#include "inodes.h"

#define VF_NORMAL   0
#define VF_PIPE     1

typedef struct _VFile VFile;
struct _VFile {
    int                 f_type;
    IndexNode           *f_inode;
    int                 f_refs;
    int                 f_mode;
    off_t               f_seek;
    void                *f_pipe;
    ListEntity          f_link;
};

void
init_vfiles();

VFile *
alloc_vfile();

void
release_vfile(VFile *file);

VFile *
dup_vfile(VFile *file);

int
map_vfile(VFile *file);

#endif // __VFILE_H__
