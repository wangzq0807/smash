#ifndef __IRQ_H__
#define __IRQ_H__

#include "x86.h"

/* 中断向量号 */
#define IRQ_PAGE        14              // 页故障

#define IRQ_HW_BEG      0x20            // 硬件中断号开始
#define IRQ_TIME        IRQ_HW_BEG
#define IRQ_KEYBOARD    IRQ_HW_BEG + 1
#define IRQ_SLAVE       IRQ_HW_BEG + 2
#define IRQ_SERIAL2     IRQ_HW_BEG + 3
#define IRQ_SERIAL1     IRQ_HW_BEG + 4
#define IRQ_LPT         IRQ_HW_BEG + 5
#define IRQ_FLOPPY      IRQ_HW_BEG + 6
#define IRQ_RELTIME     IRQ_HW_BEG + 7
#define IRQ_PS2         IRQ_HW_BEG + 12
#define IRQ_DISK        IRQ_HW_BEG + 14
#define IRQ_SYSCALL     0x80
#define IRQ_IGNORE      0xff

#ifndef __INTR_S__
typedef struct _X86Desc X86Desc;
extern X86Desc idt_table[256];

typedef struct _IrqFrame IrqFrame;
struct _IrqFrame {
    int     if_ES;  // NOTE: insw会用到这个寄存器
    int     if_DS;
    /* 下面是pushal保存的寄存器 */
    int     if_EDI;
    int     if_ESI;
    int     if_EBP;
    int     if_OSP;
    int     if_EBX;
    int     if_EDX;
    int     if_ECX;
    int     if_EAX;
    /* 中断号 */
    int     if_irqno;
    /* 当段错误时由硬件入栈(8-14) */
    int     if_errno;
    /* 由硬件入栈 */
    int     if_EIP;
    int     if_CS;
    int     if_EFLAGS;
    /* 系统特权级改变时入栈 */
    int     if_ESP;
    int     if_SS;
};

typedef int (*trap_func)(IrqFrame *);

void
setup_idt();

void
set_trap_handler(int num, trap_func f);

#endif  // __INTR_S__

#endif // __IRQ_H__