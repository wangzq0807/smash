#ifndef __MEMORY_H__
#define __MEMORY_H__
#include "sys/types.h"

#define PAGE_LOG_SIZE   12
#define PAGE_SIZE     ( 1 << PAGE_LOG_SIZE)
#define PAGE_CEILING(addr)    (((addr) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define PAGE_FLOOR(addr)      ((addr) & ~(PAGE_SIZE - 1))

#define LOG_SIZE(val) ({    \
    int32_t n = (val) - 1;  \
    int32_t ret = 0;        \
    while (n > 0) {         \
        n = n >> 1;         \
        ret += 1;           \
    }                       \
    ret;                    \
})

void
init_memory(uint32_t start, uint32_t end);

//============
// 物理页面管理
//============
uint32_t
alloc_pypage(void);

int
get_pypage_refs(uint32_t page);

int
add_pypage_refs(uint32_t page);

int
release_pypage(uint32_t page);

uint32_t
get_free_space(void);

void
pypage_copy(uint32_t pydst, uint32_t pysrc, size_t num);

//=================
// 虚拟内存管理
// 1 - 4M的一一映射
//=================
void *
alloc_vm_page();

void
release_vm_page(void *addr);

void
map_vm_page(uint32_t linaddr, uint32_t pyaddr);

//====================================
// 静态内存分配
// 初始化时,0 - 1M已完成跟物理地址的一一映射，
// 我们要在这块内存中存放内存管理的数据结构
// 实际可供使用的空间是end_kernel - 640K
//====================================
uint32_t
alloc_spage();


#endif
