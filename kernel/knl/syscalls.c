
#include "sys/syscalls.h"
#include "log.h"


#undef __SYSCALLS_H__
#undef SYSCALLS_BEGIN
#undef SYSCALLS_END
#undef SYSCALL
// 注册系统调用
#define REGIST_SYSCALL
#include "sys/syscalls.h"

#ifdef KLOG_ENABLE
    #undef __SYSCALLS_H__
    #undef SYSCALLS_BEGIN
    #undef SYSCALLS_END
    #undef SYSCALL
    #undef REGIST_SYSCALL
    // 定义系统调用名称
    #define REGIST_SYSCALL_NAME
    #include "sys/syscalls.h"
#endif


int sys_none(IrqFrame *irq) { return 0 ;}
// int sys_exit(IrqFrame *irq, int code) { return 0 ;}
// int sys_fork(IrqFrame *irq) { return 0 ;}
// int sys_read(IrqFrame *irq, const char *, ) { return 0 ;}
// int sys_write(IrqFrame *irq) { return 0 ;}
// int sys_open(IrqFrame *irq) { return 0 ;}
// int sys_close(IrqFrame *irq) { return 0 ;}
// int sys_waitpid(IrqFrame *irq) { return 0 ;}
// int sys_creat(IrqFrame *irq) { return 0 ;}
// int sys_link(IrqFrame *irq) { return 0 ;}
// int sys_unlink(IrqFrame *irq) { return 0 ;}
// int sys_execve(IrqFrame *irq) { return 0 ;}
// int sys_chdir(IrqFrame *irq) { return 0 ;}
int sys_time(IrqFrame *irq) { return 0 ;}
// int sys_mknod(IrqFrame *irq) { return 0 ;}
// int sys_chmod(IrqFrame *irq) { return 0 ;}
int sys_chown(IrqFrame *irq) { return 0 ;}
