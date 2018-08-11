#include "vfile.h"

#define     MAX_FILES   256
ListEntity  files[MAX_FILES];
ListHead    free_files;

void
init_files()
{
    for (int i = 0; i < MAX_FILES; ++i) {
        push_back(&free_files, &files[i]);
    }
}

File *
get_empty_file()
{
    ListEntity *entity = pop_front(&free_files);
    if (entity != NULL) {
        File *file = TO_INSTANCE(entity, File, f_link);
        file->f_refs = 0;
        file->f_mode = 0;
        file->f_seek = 0;
        return file;
    }
    return NULL;
}

File *
add_file_refs(File *file)
{
    file->f_refs += 1;
    return file;
}

void
release_file(File *file)
{
    file->f_refs -= 1;
    if (file->f_refs == 0) {
        push_back(&free_files, &file->f_link);
    }
}
