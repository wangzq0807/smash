#ifndef __SYSCALL_H__
#define __SYSCALL_H__
#include "defs.h"
#include "irq.h"

extern int knl_fork(struct IrqFrame *irq);
extern int knl_print(struct IrqFrame *irq);
extern int knl_exec(struct IrqFrame *irq);

struct TrapsCall{
    uint32_t    sc_num;
    trap_func   sc_func;
};

struct TrapsCall syscall[] = {
    {0, knl_fork},
    {0, knl_print},
    {0, knl_exec},
};

#endif