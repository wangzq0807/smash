#include "memory.h"
#include "asm.h"
#include "string.h"
#include "sys/types.h"

#define PAGE_LOG_SIZE   12
#define PAGE_SIZE     (1 << PAGE_LOG_SIZE)
#define PAGE_INT_SIZE         (PAGE_SIZE/sizeof(int))
#define PAGE_CEILING(addr)    (((addr) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define PAGE_FLOOR(addr)      ((addr) & ~(PAGE_SIZE - 1))
#define PAGE_MARK(addr)         ((addr) & (PAGE_SIZE - 1))

#define PAGE_PRESENT        1
#define PAGE_WRITE          2
#define PAGE_USER           1 << 2
#define PAGE_ACCESSED       1 << 5
#define PAGE_DIRTY          1 << 6

#define PDT     (0)
#define PT1     (PAGE_SIZE)

void init_memory()
{
    memset(PDT, 0, PAGE_SIZE);
    // 开启分页,映射前4M物理内存
    volatile pdt_t pdt = (pdt_t)PDT;
    volatile pt_t pt1 = (pt_t)PT1;
    size_t addr = 0;
    pdt[0] = PAGE_FLOOR((size_t)pt1) | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
    for (int i = 0; i < 256; ++i) {
        pt1[i] = PAGE_FLOOR(addr) | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
        addr += PAGE_SIZE;
    }
    load_pdt(pdt);
}

void map_mem(void *src, void *dst, int size)
{

}