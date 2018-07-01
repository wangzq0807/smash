#ifndef __SYSCALL_H__
#define __SYSCALL_H__
#include "defs.h"
#include "irq.h"

extern int knl_fork(struct IrqFrame *irq);
extern int knl_print(struct IrqFrame *irq);
extern int knl_exec(struct IrqFrame *irq);

#endif