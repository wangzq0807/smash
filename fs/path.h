#ifndef __PATH_H__
#define __PATH_H__
#include "sys/types.h"
#include "fsdefs.h"
typedef struct _IndexNode IndexNode;

typedef struct _Direction Direction;
struct _Direction {
    ino_t       dr_inode;
    char        dr_name[FILENAME_LEN+1];
};

IndexNode *
name_to_dirinode(const char *pathname, const char **basename);

IndexNode *
name_to_inode(const char *pathname);

ino_t
search_file(IndexNode *dirinode, const char *fname, int len);

int
add_file_entry(IndexNode *dirinode, const char *fname, IndexNode *inode);

int
rm_file_entry(IndexNode *dinode, const char *fname);

int
change_dir(const char *pathname);

int
make_dir(const char *pathname, int mode);

int
rm_dir(const char *pathname);

#endif // __PATH_H__