#include "path.h"
#include "fsdefs.h"
#include "inodes.h"
#include "sys/stat.h"
#include "string.h"

int
file_append(struct IndexNode *inode, void *buf, int len)
{
    return 0;
}

int
file_open(const char *pathname, int flags)
{
    struct IndexNode *drinode = dirname_to_inode(pathname);
    if (drinode == NULL) 
        return -1;

    return 0;
};

int
file_create(const char *pathname, int mode)
{
    struct IndexNode *drinode = dirname_to_inode(pathname);
    if (drinode == NULL) 
        return -1;
    const char *fname = file_name(pathname);
    struct Direction dir;
    memcpy(dir.dr_name, fname, strlen(fname));

    struct IndexNode *inode = alloc_inode(1);
    inode->in_inode.in_file_mode = S_IFREG | S_IRUSR | S_IWUSR;
    inode->in_inode.in_num_links = 1;

    dir.dr_inode = inode->in_inum;
    file_append(drinode, &dir, sizeof(struct Direction));
    release_inode(inode);
    
    return 0;
}
