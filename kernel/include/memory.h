#ifndef __MEMORY_H__
#define __MEMORY_H__
#include "sys/types.h"

#define PAGE_LOG_SIZE   12
#define PAGE_SIZE     (1 << PAGE_LOG_SIZE)
#define PAGE_CEILING(addr)    (((addr) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define PAGE_FLOOR(addr)      ((addr) & ~(PAGE_SIZE - 1))

void
init_memory(uint32_t start, uint32_t end);

//============
// 用户态物理页面管理
// 分配1M以上(由start决定)的物理内存,允许释放
// 内核数据结构应该放在640K以下,永不释放
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

//=========================================
// 虚拟内存管理
// 1 - 4M的一一映射
// 0x
// 0xFFFFE000 - 4G : 临时页表
//=========================================
void *
alloc_vm_page();

void
release_vm_page(void *addr);

void
map_vm_page(uint32_t linaddr, uint32_t pyaddr);

void
unmap_vm_page(uint32_t linaddr);

void
switch_vm_page(pde_t *cur_pdt, pde_t *new_pdt);
//====================================
// 静态内存分配
// 初始化时,0 - 1M已完成跟物理地址的一一映射，
// 我们要在这块内存中存放内存管理的数据结构
// 实际可供使用的空间是end_kernel - 636K
//====================================
uint32_t
alloc_spage();


#endif
