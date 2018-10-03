#include "vfile.h"
#include "list.h"

#define     MAX_FILES   256
ListEntity  files[MAX_FILES];
ListHead    free_files;

void
init_vfiles()
{
    for (int i = 0; i < MAX_FILES; ++i) {
        push_back(&free_files, &files[i]);
    }
}

VFile *
alloc_vfile()
{
    ListEntity *entity = pop_front(&free_files);
    if (entity != NULL) {
        VFile *file = TO_INSTANCE(entity, VFile, f_link);
        file->f_refs = 1;
        file->f_mode = 0;
        file->f_seek = 0;
        return file;
    }
    return NULL;
}

VFile *
add_vfile_refs(VFile *file)
{
    file->f_refs += 1;
    return file;
}

void
release_vfile(VFile *file)
{
    if (file == NULL);

    file->f_refs -= 1;
    if (file->f_refs == 0) {
        push_back(&free_files, &file->f_link);
    }
}

VFile *
dup_vfile(VFile *file)
{
    if (file == NULL)   return NULL;

    VFile *ret = alloc_vfile();
    file->f_inode->in_refs++;
    ret->f_inode = file->f_inode;
    ret->f_refs = file->f_refs;
    ret->f_mode = file->f_mode;
    ret->f_seek = file->f_seek;
    return ret;
}
