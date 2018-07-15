#ifndef __VFILE_H__
#define __VFILE_H__
#include "sys/types.h"

typedef struct _IndexNode IndexNode;

typedef struct _FileEntity FileEntity;
struct _FileEntity {
    int                 fe_mode;
    file_offset_t       fe_seek;
    dev_t               fe_dev;
    ino_t               fe_inode;
};

#define // __VFILE_H__
