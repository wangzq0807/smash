#include "memory.h"
#include "arch/irq.h"
#include "arch/task.h"
#include "arch/page.h"
#include "log.h"
#include "fs/file.h"
#include "fs/vfile.h"

vm_t
sys_mmap(IrqFrame *irq, vm_t addr, size_t length, int prot, int flags,
                  int fd, off_t offset)
{
    return mm_vfile(addr, length, fd, offset);
}

vm_t
sys_sbrk(IrqFrame *irq, int sz)
{
    return grow_user_vm(sz);
}
