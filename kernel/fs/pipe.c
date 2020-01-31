#include "pipe.h"
#include "memory.h"
#include "vfile.h"
#include "arch/task.h"
#include "asm.h"
#include "string.h"
#include "lib/log.h"
#include "lib/ringbuf.h"
#include "defs.h"

#define QUEUE_LEN    1024

typedef struct _Pipe Pipe;
struct _Pipe {
    VFile   *p_rdpipe;
    VFile   *p_wrpipe;
    Task*   p_rdwait;
    Task*   p_wrwait;
    RingBuf p_ringbuf;
    char    p_buf[QUEUE_LEN];
};

int
alloc_pipe(int *rdfd, int *wrfd)
{
    Pipe *pipe = (Pipe *)vm_alloc();
    VFile *vfile1 = alloc_vfile();
    VFile *vfile2 = alloc_vfile();

    int fd1 = map_vfile(vfile1);
    if (fd1 == -1) {
        release_vfile(vfile1);
        release_vfile(vfile2);
        return -1;
    }
    int fd2 = map_vfile(vfile2);
    if (fd2 == -1) {
        release_vfile(vfile1);
        release_vfile(vfile2);
        return -1;
    }
    vfile1->f_type = VF_PIPE;
    vfile1->f_pipe = pipe;
    vfile2->f_type = VF_PIPE;
    vfile2->f_pipe = pipe;
    pipe->p_rdpipe = vfile1;
    pipe->p_wrpipe = vfile2;
    pipe->p_rdwait = NULL;
    pipe->p_wrwait = NULL;
    ringbuf_init(&pipe->p_ringbuf, (uint8_t*)pipe->p_buf, QUEUE_LEN);

    *rdfd = fd1;
    *wrfd = fd2;
    return 0;
}

size_t
pipe_read(void *pipeptr, void *buf, int size)
{
    int ret = 0;
    Pipe *pipe = (Pipe*)pipeptr;
    // KLOG(DEBUG, "pipe_read %x %x %x ", pipe->p_head, pipe->p_tail, size);
    if (ringbuf_is_empty(&pipe->p_ringbuf)) {
        pipe->p_rdwait = current_task();
        sleep(pipe->p_rdwait);
    }
    ret = ringbuf_get(&pipe->p_ringbuf, (uint8_t*)buf, size);
    KLOG(DEBUG, "pipe_read 0x%x \n", ret);

    wakeup(pipe->p_wrwait);
    pipe->p_wrwait = NULL;

    return ret;
}

size_t
pipe_write(void *pipeptr, const void *buf, int size)
{
    size_t ret = 0;
    Pipe *pipe = (Pipe*)pipeptr;
    // KLOG(DEBUG, "pipe_write %x %x %x ", pipe->p_head, pipe->p_tail, size);
    ret = ringbuf_put(&pipe->p_ringbuf, (uint8_t*)buf, size);
    KLOG(DEBUG, "pipe_write 0x%x \n", ret);

    wakeup(pipe->p_rdwait);
    pipe->p_rdwait = NULL;
    return ret;
}

int
close_pipe(void *pipeptr, void *file)
{
    Pipe *pipe = (Pipe*)pipeptr;
    if (pipe->p_rdpipe == file)
        pipe->p_rdpipe = NULL;
    else if (pipe->p_wrpipe == file)
        pipe->p_wrpipe = NULL;

    if (pipe->p_rdpipe == NULL && pipe->p_wrpipe == NULL) {
        KLOG(DEBUG, "release_vm ");
        vm_free(pipe);
    }
    return 0;
}

