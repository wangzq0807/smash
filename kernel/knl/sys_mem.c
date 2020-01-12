#include "memory.h"
#include "arch/irq.h"
#include "arch/task.h"
#include "arch/page.h"
#include "lib/log.h"
#include "fs/file.h"
#include "fs/vfile.h"

vm_t
sys_mmap(IrqFrame *irq, vm_t addr, size_t length, int prot, int flags,
                  int fd, off_t offset)
{
    return vm_map_file(addr, length, fd, offset);
}

vm_t
sys_sbrk(IrqFrame *irq, int sz)
{
    return vm_user_grow(sz);
}
