#ifndef __PATH_H__
#define __PATH_H__
#include "sys/types.h"
#include "fsdefs.h"
typedef struct _IndexNode IndexNode;

typedef struct _Direction Direction;
struct _Direction {
    ino_t       dr_inode;
    char        dr_name[FILENAME_LEN];
};

IndexNode *
name_to_dirinode(const char *pathname, const char **basename);

IndexNode *
name_to_inode(const char *pathname);

ino_t
search_file(IndexNode *inode, const char *name, int len);

#endif // __PATH_H__