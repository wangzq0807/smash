#ifndef __ASM_H__
#define __ASM_H__

#include "types.h"

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

static inline void ljmp(uint32_t seg, vm_t offset) {
    struct { uint32_t o; uint32_t s; } laddr;
    laddr.o = offset;
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
    __asm__ volatile (
        "movl %0, %%cr3 \n"
        : :"r"(pdt)
    );
}

static inline size_t get_cr2() {
    register size_t cr2;
    __asm__ volatile (
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

static inline void smash_memory() {
    __asm__ volatile("nop":::"memory");
}

#endif // __IO_H__
