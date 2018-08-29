#ifndef __SYSCALL_H__
#define __SYSCALL_H__
#include "sys/types.h"
#include "irq.h"

extern int knl_fork(IrqFrame *irq);
extern int knl_print(IrqFrame *irq);
extern int knl_exec(IrqFrame *irq);

typedef struct _TrapCall TrapCall;
struct _TrapCall {
    uint32_t    sc_num;
    trap_func   sc_func;
};

TrapCall syscall[] = {
    {0, knl_fork},
    {0, knl_print},
    {0, knl_exec},
};

#endif