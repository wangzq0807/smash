#include "irq.h"
#include "task.h"
#include "asm.h"
#include "log.h"
#include "syscall.h"
#include "page.h"

/* 中断描述符表 */
struct X86Desc idt_table[256] = { 0 };
struct X86DTR idt_ptr = { 0 };

#define DECL_TRAP_FUNC(num) extern void trap##num();
DECL_TRAP_FUNC(IRQ_PAGE)
DECL_TRAP_FUNC(IRQ_TIME)
DECL_TRAP_FUNC(IRQ_DISK)
DECL_TRAP_FUNC(IRQ_SYSCALL)
DECL_TRAP_FUNC(IRQ_IGNORE)

#define TRAP_FUNC(num) trap##num

struct TrapsCall{
    uint32_t    sc_num;
    trap_func   sc_func;
};

struct TrapsCall syscall[] = {
    {0, knl_fork},
    {0, knl_print},
    {0, knl_exec},
};

struct TrapsCall trapcall[48] = { 0 };

void on_timer_handler(struct IrqFrame *irqframe);
void on_ignore_handler(struct IrqFrame *irqframe);

/* 中断门: 中断正在处理时,IF清0,从而屏蔽其他中断 */
static void
set_intr_gate(int32_t num, void *func_addr, int32_t dpl) {
    const uint32_t selector = KNL_CS;
    const uint32_t offset = (uint32_t)func_addr;
    idt_table[num].d_low = (selector << 16) | (offset & 0xFFFF);
    idt_table[num].d_high = (offset & 0xFFFF0000) | GATE_INTR_FLAG | (dpl<<13);
}

/* 陷阱门: 中断正在处理时,IF不清零,可响应更高优先级的中断 */
static void
set_trap_gate(int32_t num, void *func_addr, int32_t dpl) {
    const uint32_t selector = KNL_CS;
    const uint32_t offset = (uint32_t)func_addr;
    idt_table[num].d_low = (selector << 16) | (offset & 0xFFFF);
    idt_table[num].d_high = (offset & 0xFFFF0000) | GATE_TRAP_FLAG | (dpl<<13);
}

void
setup_idt()
{
    /* 设置默认中断 */
    for (int32_t i = 0; i < 256; ++i) {
        set_trap_gate(i, &TRAP_FUNC(IRQ_IGNORE), KNL_DPL);
    }
    set_trap_gate(IRQ_PAGE, &TRAP_FUNC(IRQ_PAGE), USR_DPL);
    /* 设置时钟中断 */
    set_intr_gate(IRQ_TIME, &TRAP_FUNC(IRQ_TIME), KNL_DPL);
    set_trap_gate(IRQ_DISK, &TRAP_FUNC(IRQ_DISK), KNL_DPL);
    /* 系统调用 */
    set_trap_gate(IRQ_SYSCALL, &TRAP_FUNC(IRQ_SYSCALL), USR_DPL);
    /* 重新加载idt */
    const uint32_t base_addr = (uint32_t)(&idt_table);
    idt_ptr.r_limit = 256*8 -1;
    idt_ptr.r_addr = base_addr;
    lidt(&idt_ptr);
}

void
on_all_irq(struct IrqFrame irqframe)
{
    /* 设置8259A的OCW2,发送结束中断命令 */
    outb(0x20, 0x20);
    outb(0x20, 0xA0);

    switch (irqframe.if_irqno) {
        case IRQ_PAGE: {
            print(" irq_page ");
            on_page_fault(&irqframe);
            break;
        }
        case IRQ_TIME: {
            on_timer_handler(&irqframe);
            break;
        }
        case IRQ_SYSCALL: {
            irqframe.if_EAX = syscall[irqframe.if_EAX].sc_func(&irqframe);
            break;
        }
        case IRQ_DISK: {
            trapcall[IRQ_DISK].sc_func(&irqframe);
            break;
        }
        default: on_ignore_handler(&irqframe); break;
    }
    /***************************
     * 防止irqframe被优化掉
     * NOTE:gcc优化时，发现irqframe没有被用到，为了节省堆栈空间，
     * 很可能覆盖irqframe中的值
     ***************************/
    smash_memory();
}

void
on_timer_handler(struct IrqFrame *irqframe)
{
    switch_task();
}

void
on_ignore_handler(struct IrqFrame *irqframe)
{
    print(" ignore ");
    printx(irqframe->if_EIP);
}

int
knl_print(struct IrqFrame *irqframe)
{
    print("C");
    return 0;
}

void set_trap_handler(int num, trap_func f)
{
    trapcall[num].sc_func = f;
}