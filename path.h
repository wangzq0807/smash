#ifndef __PATH_H__
#define __PATH_H__
#include "defs.h"
#include "fs.h"

struct Direction {
    uint16_t    dr_inode;
    char        dr_name[FILENAME_LEN];
};

uint32_t
name_to_inode(const char *name);

#endif // __PATH_H__