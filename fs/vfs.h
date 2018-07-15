#ifndef __VFILE_H__
#define __VFILE_H__
#include "sys/types.h"

typedef struct _File File;
struct _File {
    int                 fe_mode;
    off_t               fe_seek;
    dev_t               fe_dev;
    ino_t               fe_inode;
};

#endif // __VFILE_H__
