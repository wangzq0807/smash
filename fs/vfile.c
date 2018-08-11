#include "vfile.h"

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

static VFile *
_get_empty_vfile()
{
    ListEntity *entity = pop_front(&free_files);
    if (entity != NULL) {
        VFile *file = TO_INSTANCE(entity, VFile, f_link);
        file->f_refs = 0;
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
    file->f_refs -= 1;
    if (file->f_refs == 0) {
        push_back(&free_files, &file->f_link);
    }
}

VFile *
vfile_open(const char *pathname, int flags, int mode)
{
    return _get_empty_vfile();
}