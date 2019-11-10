#ifndef __ASM_H__
#define __ASM_H__

#include "sys/types.h"

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile (
        "outb %%al, %%dx\n"
        : : "a"(value), "d"(port)
    );
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret = 0;
    __asm__ volatile (
        "inb %%dx, %%al"
        :"=a"(ret) : "d"(port)
    );
    return ret;
}

static inline void outsw(uint16_t port, void *buffer, uint32_t cnt) {
    __asm__ volatile (
        "cld \n"
        "rep outsw \n"
        : : "S"(buffer), "c"(cnt), "d"(port)
    );
}

static inline void insw(uint16_t port, void *buffer, uint32_t cnt) {
    __asm__ volatile (
        "cld \n"
        "rep insw \n"
        : : "D"(buffer), "c"(cnt), "d"(port)
    );
}

static inline void cli() {
    __asm__ volatile ("cli");
}

static inline void sti() {
    __asm__ volatile ("sti");
}

static inline void lidt(void* idtr) {
    __asm__ volatile (
        "lidt (%0) \n"
        : :"r"(idtr)
    );
}

static inline void lgdt(void* gdtr) {
    __asm__ volatile (
        "lgdt (%0) \n"
        : :"r"(gdtr)
    );
}

static inline void ltr(uint32_t ltr) {
    __asm__ volatile (
        "ltr %%ax \n"
        : :"a"(ltr)
    );
}

static inline void lldt(uint32_t ldt) {
    __asm__ volatile (
        "lldt %%ax \n"
        : :"a"(ldt)
    );
}

static inline void ljmp(uint32_t seg) {
    struct { uint32_t o; uint32_t s; } laddr;
    laddr.o = 0;
    laddr.s = seg;
    __asm__ volatile (
        "ljmp *%0 \n"
        : :"m"(laddr.o), "m"(laddr.s)
    );
}

static inline void invlpg(vm_t ptr) {
    __asm__ volatile (
        "invlpg (%0) \n"
        : :"r"(ptr)
    );
}

// pdt : 页表的物理地址
static inline void load_pdt(pdt_t pdt) {
    __asm__ volatile(
        "movl %0, %%cr3 \n"
        : :"r"(pdt)
    );
}

static inline pdt_t get_pdt() {
    register pdt_t ret;
    __asm__ volatile(
        "movl %%cr3, %0 \n"
        :"=r"(ret)
    );
    return ret;
}

static inline size_t get_cr2() {
    register size_t cr2;
    __asm__ volatile(
        "movl %%cr2, %0"
        :"=r"(cr2)
    );
    return cr2;
}

static inline void enable_paging() {
    __asm__ volatile (
        "movl %%cr0, %%eax \n"
        "orl $0x80000000, %%eax \n"
        "movl %%eax, %%cr0 \n"
        :::"eax"
    );
}

// 重新加载cs和ds, gdt或ldt被修改时用到
static inline void reload_sregs(uint16_t cs, uint16_t ds) {
    __asm__ volatile (
        "pushl %%eax \n"
        "pushl $1f \n"
        "retf \n"
        "1: movw %%bx, %%ds \n"
        "movw %%bx, %%es \n"
        "movw %%bx, %%fs \n"
        "movw %%bx, %%gs \n"
        "movw %%bx, %%ss \n"
        : :"a"(cs), "b"(ds) :"esp"
    );
}

/* 构造一个假的中断帧,然后返回
    ---------
    |  ds   |
    ---------
    | stack |
    ---------
    |  cs   |
    ---------
    |  ip   |
    ---------
 */
static inline void switch_to_user(
    uint32_t code_sel, uint32_t data_sel, void* user_stack, void* entry) {
    __asm__ volatile (
        "pushl %0 \n"
        "pushl %1 \n"
        "pushf \n"
        "pushl %2 \n"
        "pushl %3 \n"
        "iret"
        // NOTE:编译器太傻了，如果下面使用内存变量的话，会使用esp来寻址，而esp自身会被push指令所修改
        : :"r"(data_sel), "r"(user_stack), "r"(code_sel), "r"(entry)
        : "esp"
    );
}

typedef struct _Task Task;
static inline Task *current_task(void) {
    Task *cur = NULL;
    __asm__ volatile (
        "movl %%esp, %%eax \n"
        "sub $1, %%eax \n"
        "andl $0xfffff000, %%eax \n"
        "test %%eax, %%eax \n"
        "jz 1f \n"
        "movl (%%eax), %%eax \n"
        "1:"
        :"=a"(cur)
    );
    return cur;
}

static inline uint32_t atomic_swap(uint32_t *org, uint32_t new_val) {
    __asm__ volatile (
        "lock xchg %0, %1 \n"
        :"+m"(*org), "+r"(new_val)
    );
    return new_val;
}

static inline uint32_t lock(uint32_t *val) {
    return atomic_swap(val, 1);
}

static inline void unlock(uint32_t *val) {
    *val = 0;
}

static inline void smash_memory() {
    __asm__ volatile("nop":::"memory");
}

static inline int call_syscall(void *irq, uint32_t cnt, uint32_t addr, void *func) {
    register int ret = 0;
    __asm__ volatile (
        // 参数个数为0的情况
        "cmpl $0, %1 \n"
        "jne 0f \n"
        "pushl %4 \n"
        "call *%3 \n"
        "popl %4 \n"
        "jmp 4f \n"
        // 有多个参数的情况
        "0: movl %1, %%ecx \n"      // ecx用来统计已复制的参数个数
        "1: subl $1, %%ecx \n"
        "pushl (%2, %%ecx, 4) \n"   // 复制用户空间的一个参数到内核栈
        "cmpl $0, %%ecx \n"
        "jne 1b \n"

        "2:pushl %4 \n"
        "call *%3 \n"
        "popl %4 \n"

        "lea (,%1, 4), %2\n"        // 假定int的宽度为4字节
        "addl %2, %%esp \n"         // 所有参数出栈
        "4: \n"
        :"+a"(ret)  // NOTE: eax不能被"r"使用，否则有可能得不到正确返回值
        :"ebx"(cnt), "r"(addr+4), "r"(func), "r"(irq) // NOTE: eax, ecx, edx的值要由调用者负责保存
        :"ecx", "esp"
    );
    return ret;
}

#endif // __IO_H__
