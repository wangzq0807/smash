#include "x86.h"
#include "asm.h"
#include "lib/log.h"

/* 全局描述符表 */
X86Desc gdt_table[7] = { 0 };
X86DTR gdt_ptr = { 0 };

/* 当前tss */
uint32_t current_tss = 1;
X86Desc  ldt[3];
// 限长
#define LDT_LIMIT   (3*8)
#define TSS_LIMIT   103

void
setup_segment_desc(X86Desc* desc, uint32_t base, uint32_t limit, int type, int dpl, int prop)
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

void
setup_gdt()
{
    setup_code_desc(&gdt_table[KNL_CS>>3], 0, 0xFFFFF, KNL_DPL);
    setup_data_desc(&gdt_table[KNL_DS>>3], 0, 0xFFFFF, KNL_DPL);

    /* 重新加载gdt */
    const uint32_t base_addr = (uint32_t)(&gdt_table);
    gdt_ptr.r_limit = 7*8 -1;
    gdt_ptr.r_addr = base_addr;
    lgdt(&gdt_ptr);
    reload_sregs(KNL_CS, KNL_DS);
}

void
switch_tss(X86TSS *tss)
{
    if (current_tss == 1) {
        setup_tss_desc(&gdt_table[KNL_TSS2>>3], (uint32_t)tss, TSS_LIMIT);
        current_tss = 2;
        ljmp(KNL_TSS2, 0);
    }
    else {
        setup_tss_desc(&gdt_table[KNL_TSS1>>3], (uint32_t)tss, TSS_LIMIT);
        current_tss = 1;
        ljmp(KNL_TSS1, 0);
    }
}

void
start_first_task(X86TSS *tss, void *func)
{
    setup_code_desc(&ldt[1], 0, 0xFFFFF, USR_DPL);
    setup_data_desc(&ldt[2], 0, 0xFFFFF, USR_DPL);
    setup_tss_desc(&gdt_table[KNL_TSS1>>3], (uint32_t)tss, TSS_LIMIT);
    setup_ldt_desc(&gdt_table[KNL_LDT>>3], (uint32_t)ldt, LDT_LIMIT);
    /* 跳转到用户空间:任务一 */
    ltr(KNL_TSS1);
    lldt(KNL_LDT);
    switch_to_user(USR_CS, USR_DS, (void *)tss->t_ESP, func);
}

void
dump_tss(X86TSS *tss)
{
    KLOG(DEBUG, "dumptss cs:%x, eip:%x, ss:%x, esp:%x, ss0:%x, esp0:%x \n",
        tss->t_CS, tss->t_EIP, tss->t_SS, tss->t_ESP, tss->t_SS_0, tss->t_ESP_0);
}
