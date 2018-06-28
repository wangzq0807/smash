#include "arch.h"
#include "asm.h"
#include "log.h"

/* 中断描述符 */
struct X86Desc idt_table[256] = { 0 };
struct X86DTR idt_ptr = { 0 };
/* 全局描述符 */
struct X86Desc gdt_table[7] = { 0 };
struct X86DTR gdt_ptr = { 0 };
/* 任务描述符 */
uint32_t current = 1;
/* 任务一 */
struct X86TSS tss1 = { 0 };
uint8_t tss1_kernal_stack[4096] = { 0 };
uint8_t tss1_user_stack[4096] = { 0 };
struct X86Desc tss1_ldt[3] = { 0 };
/* 任务二 */
struct X86TSS tss2 = { 0 };
uint8_t tss2_kernal_stack[4096] = { 0 };
uint8_t tss2_user_stack[4096] = { 0 };
struct X86Desc tss2_ldt[3] = { 0 };

/* 局部描述符 */
struct X86Desc ldt_table_1[3] = { 0 };
struct X86DTR ldt_ptr_1 = { 0 };
struct X86Desc ldt_table_2[3] = { 0 };
struct X86DTR ldt_ptr_2 = { 0 };

extern void on_timer_intr();
extern void on_ignore_intr();

/* 中断门: 中断正在处理时,IF清0,从而屏蔽其他中断 */
void
set_intr_gate(int32_t num, void *func_addr)
{
    const uint32_t selector = KNL_CS;
    const uint32_t offset = (uint32_t)func_addr;
    idt_table[num].d_low = (selector << 16) | (offset & 0xFFFF);
    idt_table[num].d_high = (offset & 0xFFFF0000) | GATE_INTR_FLAG;
}

/* 陷阱门: 中断正在处理时,IF不清零,可响应更高优先级的中断 */
void
set_trap_gate(int32_t num, void *func_addr)
{
    const uint32_t selector = KNL_CS;
    const uint32_t offset = (uint32_t)func_addr;
    idt_table[num].d_low = (selector << 16) | (offset & 0xFFFF);
    idt_table[num].d_high = (offset & 0xFFFF0000) | GATE_TRAP_FLAG;
}

static void
_init_8259A()
{
    /* init_ICW1: 00010001b,多片级联,使用ICW4 */
    outb(0x11, 0x20);
    outb(0x11, 0xA0);
    /* init_ICW2: 中断号 0x20 - 0x27, 0x28 - 0x2f */
    outb(0x20, 0x21);
    outb(0x28, 0xA1);
    /* init_ICW3: 100b, IR2 接从片 */
    outb(0x4, 0x21);        /* 100b, IR2 接从片 */
    outb(0x2, 0xA1);        /* 接主片IR2 */
    /* init_ICW4: 普通全嵌套, 非缓冲, 非自动结束 */
    outb(0x1, 0x21);        /* 普通全嵌套, 非缓冲, 非自动结束 */
    outb(0x1, 0xA1);
}

static void 
_init_timer()
{
    /* 写入控制字, 使用方式3, 计数器0 */
    outb(0x36, 0x43);
    /* 写入计数值，设置每10ms中断一次 */
    uint8_t low = BYTE1(11930);
    outb(low, 0x40);    /* 先写低字节 */
    uint8_t high = BYTE2(11930);
    outb(high, 0x40);   /* 后写高字节 */
}

void
switch_task()
{
    if (current == 1) {
        current = 2;
        __asm__ volatile (
            "ljmp $0x28, $0 \n"
        );
    }
    else {
        current = 1;
        __asm__ volatile (
            "ljmp $0x18, $0 \n"
        );
    }
}

void
on_timer_handler()
{
    /* 设置8259A的OCW2,发送结束中断命令 */
    outb(0x20, 0x20);
    outb(0x20, 0xA0);

    // print("timer");
}

void
on_ignore_handler()
{
    /* 设置8259A的OCW2,发送结束中断命令 */
    outb(0x20, 0x20);
    outb(0x20, 0xA0);
}

static void
setup_idt()
{
    /* 设置默认中断 */
    for (int32_t i = 0; i < 256; ++i) {
        set_intr_gate(i, &on_ignore_intr);
    }
    /* 设置时钟中断 */
    set_intr_gate(INTR_TIMER, &on_timer_intr);
    /* 重新加载idt */
    const uint32_t base_addr = (uint32_t)(&idt_table);
    idt_ptr.r_limit = 256*8 -1;
    idt_ptr.r_addr = base_addr;
    lidt(&idt_ptr);
}

static void
_setup_segment_desc(struct X86Desc* desc, uint32_t base, uint32_t limit, uint8_t type, uint8_t dpl, uint8_t prop)
{
    const uint32_t SEG_LIMIT_16 = limit & 0xFFFF;        // 取limit的0-15位
    const uint32_t SEG_LIMIT_20 = limit & 0xF0000;       // 取limit的19-16位
    const uint32_t SEG_BASE_16 = base & 0xFFFF;          // 取base的0-15位
    const uint32_t SEG_BASE_24 = base & 0xFF0000;        // 取base的23-16位
    const uint32_t SEG_BASE_32 = base & 0xFF000000;      // 取base的31-24位

    const uint32_t SEG_TYPE = (type & 0xF) << 8;         // 类型
    const uint32_t SEG_DPL  = (dpl & 0xF) << 12;         // 特权级0
    const uint32_t SEG_PROP = (prop & 0xF) << 20;        // 粒度,操作数位数,等

    desc->d_low = SEG_BASE_16 << 16 | SEG_LIMIT_16;
    desc->d_high = SEG_BASE_32 | SEG_PROP | SEG_LIMIT_20 |
                    SEG_DPL | SEG_TYPE | (SEG_BASE_24 >> 16);
}

static void
_setup_code_desc(struct X86Desc* desc, uint32_t base, uint32_t limit, uint8_t dpl)
{
    const uint8_t CS_TYPE = 0xA;        // 非一致，可读
    const uint8_t CS_PROP = 0xC;        // 粒度4KB，32位操作数
    _setup_segment_desc(desc, base, limit, CS_TYPE, dpl, CS_PROP);
}

static void
_setup_data_desc(struct X86Desc* desc, uint32_t base, uint32_t limit, uint8_t dpl)
{
    const uint32_t DS_TYPE = 0x2;       // 非一致，可写
    const uint32_t DS_PROP = 0xC;       // 粒度4KB，32位操作数
    _setup_segment_desc(desc, base, limit, DS_TYPE, dpl, DS_PROP);
}

static void
_setup_tss_desc(struct X86Desc* desc, uint32_t base, uint32_t limit, uint8_t dpl)
{
    const uint32_t TSS_TYPE = 0x9;      // 非忙
    const uint32_t TSS_PROP = 0x0;      // 大于等于104字节
    _setup_segment_desc(desc, base, limit, TSS_TYPE, dpl, TSS_PROP);
}

static void
_setup_ldt_desc(struct X86Desc* desc, uint32_t base, uint32_t limit, uint8_t dpl)
{
    const uint32_t LDT_TYPE = 0x2;      // LDT
    const uint32_t LDT_PROP = 0x0;      //
    _setup_segment_desc(desc, base, limit, LDT_TYPE, dpl, LDT_PROP);
}

static void
task_1()
{
    __asm__ volatile (
        "mov $0x17, %%ax \n"
        "mov %%ax, %%ds \n"
        : : : "%eax"
    );
    while( 1 ) {
        print("ABC");
        pause();
        pause();
        pause();
    }
}

static void
task_2()
{
    __asm__ volatile (
        "mov $0x17, %%ax \n"
        "mov %%ax, %%ds \n"
        : : : "%eax"
    );
    while( 1 ) {
        print("DEF");
        pause();
        pause();
        pause();
    }
}

static void
setup_gdt()
{
    const uint8_t KNL_DPL = 0x9;        // 内核段DPL : 存在，特权级0，数据段/代码段
    _setup_code_desc(&gdt_table[1], 0, 0xFFFFF, KNL_DPL);
    _setup_data_desc(&gdt_table[2], 0, 0xFFFFF, KNL_DPL);

    const uint8_t TSS_DPL = 0x8;        // 任务状态段DPL : 存在，特权级0，系统段
    _setup_tss_desc(&gdt_table[3], (uint32_t)&tss1, 103, TSS_DPL);
    const uint8_t LDT_DPL = 0x8;        // LDT段DPL : 存在，特权级0，系统段
    _setup_ldt_desc(&gdt_table[4], (uint32_t)&tss1_ldt, 24, LDT_DPL);

    _setup_tss_desc(&gdt_table[5], (uint32_t)&tss2, 103, TSS_DPL);
    _setup_ldt_desc(&gdt_table[6], (uint32_t)&tss2_ldt, 24, LDT_DPL);

    /* 重新加载gdt */
    const uint32_t base_addr = (uint32_t)(&gdt_table);
    gdt_ptr.r_limit = 7*8 -1;
    gdt_ptr.r_addr = base_addr;
    lgdt(&gdt_ptr);
    reload_sregs(KNL_CS, KNL_DS);
}

static void
setup_ldt()
{
    const uint8_t USR_DPL = 0xF;
    _setup_code_desc(&tss1_ldt[1], 0, 0xFFFFF, USR_DPL);
    _setup_data_desc(&tss1_ldt[2], 0, 0xFFFFF, USR_DPL);
    _setup_code_desc(&tss2_ldt[1], 0, 0xFFFFF, USR_DPL);
    _setup_data_desc(&tss2_ldt[2], 0, 0xFFFFF, USR_DPL);
}

static void
setup_tss()
{
    tss1.t_SS_0 = KNL_DS;
    tss1.t_ESP_0 = (uint32_t)&tss1_kernal_stack[4095];
    tss1.t_EIP = (uint32_t)task_1;
    tss1.t_LDT = (uint32_t)0x20;

    tss2.t_SS_0 = KNL_DS;
    tss2.t_ESP_0 = (uint32_t)&tss2_kernal_stack[4095];

    tss2.t_EFLAGS = 0x200;  // NOTE: 允许中断
    tss2.t_DS = 0x17;
    tss2.t_SS = 0x17;
    tss2.t_ESP = (uint32_t)&tss2_user_stack[4095];
    tss2.t_CS = 0xF;
    tss2.t_EIP = (uint32_t)task_2;
    tss2.t_LDT = (uint32_t)0x30;
}

void
init_cpu()
{
    cli();
    setup_idt();
    setup_gdt();
    setup_ldt();
    setup_tss();

    _init_8259A();
    _init_timer();

    // /* 跳转到用户空间 */
    // ltr(0x18);
    // lldt(0x20);

    sti();

    // switch_to_user(0xF, 0x17, &tss1_user_stack[4095], task_1);
}

