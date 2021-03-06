#include "arch.h"
#include "asm.h"
#include "lib/log.h"
#include "task.h"
#include "x86.h"
#include "memory.h"
#include "page.h"
#include "utc.h"
#include "dev/char/serial.h"
#include "defs.h"

static void
setup_apic()
{
    /* init_ICW1: 00010001b,多片级联,使用ICW4 */
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    /* init_ICW2: 中断号 0x20 - 0x27, 0x28 - 0x2f */
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    /* init_ICW3: 100b, IR2 接从片 */
    outb(0x21, 0x4);        /* 100b, IR2 接从片 */
    outb(0xA1, 0x2);        /* 接主片IR2 */
    /* init_ICW4: 普通全嵌套, 非缓冲, 非自动结束 */
    outb(0x21, 0x1);        /* 普通全嵌套, 非缓冲, 非自动结束 */
    outb(0xA1, 0x1);
}

static void
setup_time()
{
    /* 写入控制字, 使用方式3, 计数器0 */
    outb(0x43, 0x36);
    /* 写入计数值，设置每10ms中断一次 */
    uint8_t low = BYTE1(11930);
    outb(0x40, low);    /* 先写低字节 */
    uint8_t high = BYTE2(11930);
    outb(0x40, high);   /* 后写高字节 */
}

void
init_isa()
{
    cli();
    setup_serial(COM_PORT1);
    setup_gdt();
    setup_idt();

    setup_apic();

    setup_time();
    setup_utc();
    sti();
}

