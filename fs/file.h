#ifndef __FILE_H__
#define __FILE_H__
#include "sys/types.h"
#include "vfile.h"

File *
file_create(const char *pathname, int flags, int mode);

File *
file_open(const char *pathname, int flags, int mode);

ssize_t
file_read(const char *pathname, void *buf, size_t count);

ssize_t
file_write(const char *pathname, const void *buf, size_t count);

int
file_close(File *fd);

#endif // __FILE_H__