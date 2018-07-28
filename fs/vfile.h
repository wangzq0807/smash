#ifndef __VFILE_H__
#define __VFILE_H__
#include "sys/types.h"

typedef struct _File File;
struct _File {
    dev_t               f_dev;
    ino_t               f_inode;
    int                 f_refs;
    int                 f_mode;
    off_t               f_seek;
};

#endif // __VFILE_H__
