#ifndef __SYSCALLS_H__
#define __SYSCALLS_H__
#include "sys/types.h"
#include "arch/irq.h"

#ifdef REGIST_SYSCALL
    #define SYSCALLS_BEGIN()        TrapCall syscalls[] = {
    #define SYSCALLS_END()          };
    #define SYSCALL(name, params)   { params, sys_##name },
#elif defined REGIST_SYSCALL_NAME
    #define SYSCALLS_BEGIN()        char* syscalls_name[] = {
    #define SYSCALLS_END()          };
    #define SYSCALL(name, params)   "sys_"#name,
#else
    typedef struct _TrapCall TrapCall;
    struct _TrapCall {
        int         sc_params;
        trap_func   sc_func;
    };

    extern TrapCall syscalls[];
    extern char* syscalls_name[];
    #define SYSCALLS_BEGIN()
    #define SYSCALLS_END()
    #define SYSCALL(name, params)   extern int sys_##name();
#endif // REGIST_SYSCALL

SYSCALLS_BEGIN()
SYSCALL(none, 0)            // 0
SYSCALL(exit, 1)            // 1
SYSCALL(fork, 0)            // 2
SYSCALL(read, 3)            // 3
SYSCALL(write, 3)           // 4
SYSCALL(open, 3)            // 5
SYSCALL(close, 1)           // 6
SYSCALL(waitpid, 4)         // 7
SYSCALL(creat, 2)           // 8
SYSCALL(link, 2)            // 9
SYSCALL(unlink, 1)          // 10
SYSCALL(execve, 3)          // 11
SYSCALL(chdir, 1)           // 12
SYSCALL(time, 0)            // 13
SYSCALL(mknod, 3)           // 14
SYSCALL(chmod, 2)           // 15
SYSCALL(chown, 3)           // 16

SYSCALL(mkdir, 2)           // 17
SYSCALL(rmdir, 1)           // 18
SYSCALL(pause, 0)           // 19
SYSCALL(getpid, 0)          // 20
SYSCALL(pipe, 1)            // 21
SYSCALL(dup, 1)             // 22
SYSCALL(mmap, 6)            // 23
SYSCALL(sbrk, 1)            // 24

SYSCALLS_END()

#endif // __SYSCALLS_H__