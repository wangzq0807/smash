#ifndef __SYSCALL_H__
#define __SYSCALL_H__
#include "defs.h"
#include "irq.h"

extern int knl_fork(IrqFrame *irq);
extern int knl_print(IrqFrame *irq);
extern int knl_exec(IrqFrame *irq);

typedef struct _TrapsCall TrapsCall;
struct _TrapsCall {
    uint32_t    sc_num;
    trap_func   sc_func;
};

TrapsCall syscall[] = {
    {0, knl_fork},
    {0, knl_print},
    {0, knl_exec},
};

#endif