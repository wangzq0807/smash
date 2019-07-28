#include "vfile.h"
#include "list.h"
#include "file.h"
#include "arch/task.h"
#include "asm.h"
#include "pipe.h"

#define     MAX_FILES   256
VFile  files[MAX_FILES];
ListHead    free_files;

void
init_vfiles()
{
    for (int i = 0; i < MAX_FILES; ++i) {
        list_push_back(&free_files, &files[i].f_link);
    }
}

VFile *
alloc_vfile()
{
    ListEntity *entity = list_pop_front(&free_files);
    if (entity != NULL) {
        VFile *file = TO_INSTANCE(entity, VFile, f_link);
        file->f_refs = 1;
        file->f_mode = 0;
        file->f_seek = 0;
        file->f_type = VF_NORMAL;
        file->f_pipe = NULL;
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
        if (file->f_type == VF_NORMAL)
            file_close(file->f_inode);
        else if (file->f_type == VF_PIPE)
            close_pipe(file->f_pipe, file);
        list_push_back(&free_files, &file->f_link);
    }
}

VFile *
dup_vfile(VFile *file)
{
    if (file == NULL)   return NULL;

    file->f_refs += 1;
    return file;
}

int
map_vfile(VFile *file)
{
    Task *cur = current_task();
    int i = 0;
    for (; i < MAX_FD; ++i) {
        if (cur->ts_filps[i] == NULL) {
            cur->ts_filps[i] = file;
            break;
        }
    }
    if (i == MAX_FD)    return -1;
    else return i;
}
