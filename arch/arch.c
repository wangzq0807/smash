#include "arch.h"
#include "asm.h"
#include "log.h"
#include "task.h"
#include "x86.h"
#include "memory.h"
#include "page.h"
#include "utc.h"

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

static void
_setup_pages()
{
    uint32_t *pdt = (uint32_t*)alloc_spage();
    uint32_t addr = 0;
    // 先将0-1M一一映射到物理内存, 1M - 4M 用于动态分配
    uint32_t *pte = (uint32_t*)alloc_spage();
    pdt[0] = PAGE_FLOOR((uint32_t)pte) | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
    for (int i = 0; i < 256; ++i) {
        pte[i] = PAGE_FLOOR(addr) | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
        addr += PAGE_SIZE;
    }
    load_cr3(pdt);
}

void
init_isa()
{
    cli();
    setup_idt();
    setup_gdt();

    _init_8259A();
    _init_timer();

    _setup_pages();
    enable_paging();

    init_utc();
    sti();
}

