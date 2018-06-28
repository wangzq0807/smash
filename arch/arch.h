#ifndef __ARCH_H__
#define __ARCH_H__
#include "defs.h"
/* 内核代码段和数据段 */
#define KNL_CS      0x8
#define KNL_DS      0x10
/*  */
#define TSK_CS      (0x8 + 0b11)
#define TSK_DS      (0x10 + 0b11)
#define TSK_TSS(n)  ((2*(n)+3) << 3)
#define TSK_LDT(n)  (TSK_TSS(n) + 8)
#define TSK_MAX     128
/* 门描述符标志 */
#define GATE_INTR_FLAG  0x8E00
#define GATE_TRAP_FLAG  0x8F00
/* 中断向量号 */
#define INTR_HW_BEG     0x20            // 硬件中断号开始
#define INTR_TIMER      INTR_HW_BEG
#define INTR_KEYBOARD   INTR_HW_BEG + 1
#define INTR_SLAVE      INTR_HW_BEG + 2
#define INTR_SERIAL2    INTR_HW_BEG + 3
#define INTR_SERIAL1    INTR_HW_BEG + 4
#define INTR_LPT        INTR_HW_BEG + 5
#define INTR_FLOPPY     INTR_HW_BEG + 6
#define INTR_RELTIME    INTR_HW_BEG + 7
#define INTR_PS2        INTR_HW_BEG + 12
#define INTR_DISK       INTR_HW_BEG + 14

struct X86Desc {
    uint32_t d_low;
    uint32_t d_high;
};

struct X86DTR {
    uint16_t r_limit;
    uint32_t r_addr;
} __attribute__((packed));  // 取消对齐优化

struct X86TSS {
    uint16_t    t_pre_TSS;

    uint32_t    t_ESP_0;
    uint16_t    t_SS_0;
    uint32_t    t_ESP_1;
    uint16_t    t_SS_1;
    uint32_t    t_ESP_2;
    uint16_t    t_SS_2;

    uint32_t    t_CR3;
    uint32_t    t_EIP;
    uint32_t    t_EFLAGS;
    uint32_t    t_EAX;
    uint32_t    t_ECX;
    uint32_t    t_EDX;
    uint32_t    t_EBX;
    uint32_t    t_ESP;
    uint32_t    t_EBP;
    uint32_t    t_ESI;
    uint32_t    t_EDI;

    uint32_t    t_ES;
    uint32_t    t_CS;
    uint32_t    t_SS;
    uint32_t    t_DS;
    uint32_t    t_FS;
    uint32_t    t_GS;
    uint32_t    t_LDT;
    uint16_t    t_Trap;
    uint16_t    t_IO_map;
};

void set_intr_gate(int32_t num, void *func_addr);
void set_trap_gate(int32_t num, void *func_addr);

void init_cpu();

#endif // __ARCH_H__