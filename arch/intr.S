#ifndef __INTR_S__
#define __INTR_S__
#include "irq.h"

#define TRAP_FUNC(num)  \
.global trap##num;      \
trap##num:              \
    pushl $0;           \
    pushl $num;         \
    jmp all_intr_request

#define TRAP_FUNC0(num) \
.global trap##num;      \
trap##num:              \
    pushl $num;         \
    jmp all_intr_request

.text

TRAP_FUNC0(IRQ_PAGE)
TRAP_FUNC(IRQ_TIME)
TRAP_FUNC(IRQ_KEYBOARD)
TRAP_FUNC(IRQ_DISK)
TRAP_FUNC(IRQ_IGNORE)
TRAP_FUNC(IRQ_SYSCALL)

/* NOTE:中断处理前要保存寄存器的值 */
all_intr_request:
    pushal
    pushl %ds
    movw $KNL_DS, %ax
    movw %ax, %ds
    call on_all_irq
    popl %ds
    popal
    addl $8, %esp    /* 中断号出栈 */
    iret

#endif  /* __INTR_S__ */
