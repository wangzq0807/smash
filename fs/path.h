#ifndef __PATH_H__
#define __PATH_H__
#include "defs.h"
#include "fsdefs.h"
typedef struct _IndexNode IndexNode;

typedef struct _Direction Direction;
struct _Direction {
    ino_t       dr_inode;
    char        dr_name[FILENAME_LEN];
};

IndexNode *
name_to_inode(const char *name, const char **remain);

static inline const char *file_name(const char *name) {
    const char *ret = name;
    for (; *name != NULL; ++name) {
        if (*name == '/')
            ret = name+1;
    }
    return ret;
}

#endif // __PATH_H__