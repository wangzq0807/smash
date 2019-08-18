#include "pipe.h"
#include "memory.h"
#include "vfile.h"
#include "arch/task.h"
#include "asm.h"
#include "string.h"
#include "log.h"

#define QUEUE_LEN    1024

typedef struct _Pipe Pipe;
struct _Pipe {
    VFile   *p_rdpipe;
    VFile   *p_wrpipe;
    int     p_head;
    int     p_tail;
    Task*   p_rdwait;
    Task*   p_wrwait;
    char    p_buf[QUEUE_LEN];
};

static int _is_empty_queue(Pipe *pipe);
static int _is_full_queue(Pipe *pipe);

int
alloc_pipe(int *rdfd, int *wrfd)
{
    Pipe *pipe = (Pipe *)alloc_vm_page();
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
    pipe->p_head = 0;
    pipe->p_tail = 0;
    pipe->p_rdwait = NULL;
    pipe->p_wrwait = NULL;

    *rdfd = fd1;
    *wrfd = fd2;
    return 0;
}

int
pipe_read(void *pipeptr, void *buf, int size)
{
    int ret = 0;
    Pipe *pipe = (Pipe*)pipeptr;
    KLOG(DEBUG, "pipe_read %x %x %x ", pipe->p_head, pipe->p_tail, size);
    if (_is_empty_queue(pipe)) {
        pipe->p_rdwait = current_task();
        sleep(pipe->p_rdwait);
    }
    if (pipe->p_head <= pipe->p_tail) {
        ret = pipe->p_tail - pipe->p_head;
        ret = MIN(ret, size);
        memcpy(buf, pipe->p_buf + pipe->p_head, ret);
        pipe->p_head += ret;
    }
    else {
        int len1 = QUEUE_LEN - pipe->p_head;
        int len2 = pipe->p_tail;
        if (size < len1) {
            ret = size;
            memcpy(buf, pipe->p_buf + pipe->p_head, ret);
        }
        else {
            size -= len1;
            memcpy(buf, pipe->p_buf + pipe->p_head, len1);
            int more = MIN(size, len2);
            memcpy(buf + len1, pipe->p_buf, more);
            pipe->p_head = more;
            ret = len1 + more;
        }
    }
    KLOG(DEBUG, "rend %x %x %x\n", pipe->p_head, pipe->p_tail, ret);

    wakeup(pipe->p_wrwait);
    pipe->p_wrwait = NULL;

    return ret;
}

int
pipe_write(void *pipeptr, const void *buf, int size)
{
    int ret = 0;
    Pipe *pipe = (Pipe*)pipeptr;
    KLOG(DEBUG, "pipe_write %x %x %x ", pipe->p_head, pipe->p_tail, size);
    if (_is_full_queue(pipe)) {
        pipe->p_wrwait = current_task();
        sleep(pipe->p_wrwait);
    }
    int limit = (pipe->p_head + QUEUE_LEN - 1) % QUEUE_LEN;
    if (pipe->p_tail <= limit) {
        ret = limit - pipe->p_tail;
        ret = MIN(ret, size);
        memcpy(pipe->p_buf + pipe->p_tail, buf, ret);
        pipe->p_tail += ret;
    }
    else {
        int len1 = QUEUE_LEN - pipe->p_tail;
        int len2 = limit;
        if (size < len1) {
            memcpy(pipe->p_buf + pipe->p_tail, buf, size);
            ret = size;
            pipe->p_tail += size;
        }
        else {
            size -= len1;
            memcpy(pipe->p_buf + pipe->p_tail, buf, len1);
            int more = MIN(size, len2);
            memcpy(pipe->p_buf, buf+len1, more);
            ret = len1 + more;
            pipe->p_tail = more;
        }
    }
    KLOG(DEBUG, "wend %x %x %x\n", pipe->p_head, pipe->p_tail, ret);

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
        release_vm_page(pipe);
    }
    return 0;
}


static int
_is_empty_queue(Pipe *pipe)
{
    if (pipe->p_head == pipe->p_tail)
        return 1;
    else
        return 0;
}

static int
_is_full_queue(Pipe *pipe)
{
    if ((pipe->p_head + 1) == pipe->p_tail) {
        return 1;
    }
    else if ((pipe->p_tail == 0) && (pipe->p_head == (QUEUE_LEN-1))) {
        return 1;
    }
    else if ((pipe->p_head == 0) && (pipe->p_tail == (QUEUE_LEN-1))) {
        return 1;
    }
    else {
        return 0;
    }
}
