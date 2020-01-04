#ifndef __MEMORY_H__
#define __MEMORY_H__
#include "sys/types.h"
#include "mem/linear.h"

extern char _LMA;
extern char _VMA;
extern char kernel_start;   // LMA
extern char boot_end;       // LMA
extern char kernel_end;     // LMA

#define VMA(val)    ((vm_t)(val) + (vm_t)&_VMA)
#define LMA(val)    ((vm_t)(val) - (vm_t)&_VMA)

void
vm_init();

void*
vm_ualloc();

void*
vm_kalloc();

int
vm_free(void*);

int
vm_is_user_space(vm_t vaddr);

/*
//============
// 用户态物理页面管理
// 分配1M以上(由start决定)的物理内存,允许释放.
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
*/

//=========================================
// 虚拟内存管理
// 1 - 4M的一一映射
// 0x
// 0xFFFFE000 - 4G : 临时页表
//=========================================
vm_t
alloc_vm_page();

void
release_vm_page(vm_t addr);

void
map_vm_page(vm_t linaddr, uint32_t pyaddr);

void
unmap_vm_page(vm_t linaddr);

void
switch_vm_page(pdt_t cur_pdt, pdt_t new_pdt);

// 分配页表
pt_t
alloc_page_table(pde_t *pde);

//====================================
// 静态内存分配
// 初始化时,0 - 1M已完成跟物理地址的一一映射，
// 我们要在这块内存中存放内存管理的数据结构
// 实际可供使用的空间是end_kernel - 636K
//====================================
uint32_t
alloc_spage();

// 文件映射(分配虚拟页面,不会实际分配物理页面)
vm_t
mm_vfile(vm_t addr, size_t length, int fd, off_t offset);

// 虚拟内存的最大限制(分配虚拟页面,不会实际分配物理页面)
size_t
grow_user_vm(int sz);


#endif
