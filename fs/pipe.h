#ifndef __PIPE_H__
#define __PIPE_H__
#include "sys/types.h"

int
alloc_pipe(int *rdfd, int *wrfd);

int
pipe_read(void *pipe, void *buf, int size);

int
pipe_write(void *pipe, const void *buf, int size);

int
close_pipe(void *pipe, void *file);

#endif // __PIPE_H__
