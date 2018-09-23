#ifndef __VFILE_H__
#define __VFILE_H__
#include "sys/types.h"
#include "inodes.h"

typedef struct _VFile VFile;
struct _VFile {
    IndexNode           *f_inode;
    int                 f_refs;
    int                 f_mode;
    off_t               f_seek;
    ListEntity          f_link;
};

void
init_vfiles();

VFile *
alloc_vfile();

VFile *
add_vfile_refs(VFile *file);

void
release_vfile(VFile *file);

VFile *
dup_vfile(VFile *file);

#endif // __VFILE_H__
