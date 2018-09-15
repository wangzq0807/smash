#include "arch/irq.h"
#include "arch/task.h"
#include "log.h"
#include "fs/file.h"
#include "sys/types.h"

int
sys_open(IrqFrame *irq, const char *filename, int flags, int mode)
{
    return 0;
}

int
sys_read(IrqFrame *irq, int fd, void *buf, size_t count)
{
    return 0;
}

int
sys_write(IrqFrame *irq, int fd, const void *buf, size_t count)
{
    return 0;
}

int
sys_close(IrqFrame *irq, int fd)
{
    return 0;
}