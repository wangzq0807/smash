#ifndef __X86_H__
#define __X86_H__

#define KNL_DPL     0
#define USR_DPL     3

/* 内核代码段和数据段 */
#define KNL_CS      0x8
#define KNL_DS      0x10
/* 只使用两个tss段，通过不断反转两个tss实现任务切换 */
#define KNL_TSS1    0x18
#define KNL_TSS2    0x20
/* 所有任务使用相同的LDT描述符 */
#define KNL_LDT     0x28

/* 用户态代码段和数据段 */
#define USR_CS      0xF
#define USR_DS      0x17

/* 门描述符标志 */
#define GATE_INTR_FLAG  0x8E00
#define GATE_TRAP_FLAG  0x8F00

#ifndef __INTR_S__

#include "defs.h"

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

/************
 * 描述符
 ************/
void
setup_segment_desc(struct X86Desc* desc, uint32_t base, uint32_t limit, int type, int dpl, int prop);

/* 代码段描述符：非一致性 */
static inline void
setup_code_desc(struct X86Desc* desc, uint32_t base, uint32_t limit, int dpl) {
    const int CS_TYPE = 0xA;        // 非一致，可读
    const int CS_PROP = 0xC;        // 粒度4KB，32位操作数
    const int CS_DPL = dpl << 1 | 0x9;  // 存在，数据段/代码段
    setup_segment_desc(desc, base, limit, CS_TYPE, CS_DPL, CS_PROP);
}

/* 数据段描述符 */
static inline void
setup_data_desc(struct X86Desc* desc, uint32_t base, uint32_t limit, int dpl) {
    const int DS_TYPE = 0x2;       // 非一致，可写
    const int DS_PROP = 0xC;       // 粒度4KB，32位操作数
    const int DS_DPL = dpl << 1 | 0x9;  // 存在，数据段/代码段
    setup_segment_desc(desc, base, limit, DS_TYPE, DS_DPL, DS_PROP);
}

/* 任务状态段描述符 */
static inline void
setup_tss_desc(struct X86Desc* desc, uint32_t base, uint32_t limit) {
    const int TSS_TYPE = 0x9;      // 非忙
    const int TSS_PROP = 0x0;      // 大于等于104字节
    const int TSS_DPL = 0x8;       // 任务状态段DPL : 存在，特权级0，系统段
    setup_segment_desc(desc, base, limit, TSS_TYPE, TSS_DPL, TSS_PROP);
}

/* 局部描述符 */
static inline void
setup_ldt_desc(struct X86Desc* desc, uint32_t base, uint32_t limit) {
    const int LDT_TYPE = 0x2;      // LDT
    const int LDT_PROP = 0x0;      //
    const int LDT_DPL = 0x8;       // LDT段DPL : 存在，特权级0，系统段
    setup_segment_desc(desc, base, limit, LDT_TYPE, LDT_DPL, LDT_PROP);
}

void
setup_gdt();

void
switch_tss(struct X86TSS *tss);

void
start_first_task(struct X86TSS *tss, void *func);

void dump_tss(struct X86TSS *tss);
#endif // __INTR_S__

#endif // __X86_H__