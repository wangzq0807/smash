
#include "sys/syscalls.h"

#ifdef __SYSCALLS_H__
#undef __SYSCALLS_H__

#undef SYSCALLS_BEGIN
#undef SYSCALLS_END
#undef SYSCALL

#define SYSCALLS_SOURCE
#include "sys/syscalls.h"

#endif  // __SYSCALLS_H__

int sys_none(IrqFrame *irq) { return 0 ;}
int sys_exit(IrqFrame *irq) { return 0 ;}
// int sys_fork(IrqFrame *irq) { return 0 ;}
int sys_read(IrqFrame *irq) { return 0 ;}
int sys_write(IrqFrame *irq) { return 0 ;}
int sys_open(IrqFrame *irq) { return 0 ;}
int sys_close(IrqFrame *irq) { return 0 ;}
int sys_waitpid(IrqFrame *irq) { return 0 ;}
int sys_creat(IrqFrame *irq) { return 0 ;}
int sys_link(IrqFrame *irq) { return 0 ;}
int sys_unlink(IrqFrame *irq) { return 0 ;}
// int sys_execve(IrqFrame *irq) { return 0 ;}
int sys_chdir(IrqFrame *irq) { return 0 ;}
int sys_time(IrqFrame *irq) { return 0 ;}
int sys_mknod(IrqFrame *irq) { return 0 ;}
int sys_chmod(IrqFrame *irq) { return 0 ;}
int sys_chown(IrqFrame *irq) { return 0 ;}
