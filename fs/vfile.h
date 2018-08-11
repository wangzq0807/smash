#ifndef __VFILE_H__
#define __VFILE_H__
#include "sys/types.h"
#include "list.h"

typedef struct _File File;
struct _File {
    dev_t               f_dev;
    ino_t               f_inode;
    int                 f_refs;
    int                 f_mode;
    off_t               f_seek;
    ListEntity          f_link;
};

void
init_files();

File *
get_empty_file();

File *
add_file_refs(File *file);

void
release_file(File *file);

#endif // __VFILE_H__
