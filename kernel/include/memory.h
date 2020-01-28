#ifndef __MEMORY_H__
#define __MEMORY_H__
#include "sys/types.h"
#include "mem/linear.h"

extern char _LMA;
extern char _VMA;
extern char kernel_start;   // LMA
extern char boot_end;       // LMA
extern char kernel_end;     // LMA

void
memory_setup();

void*
vm_alloc();

int
vm_free(void* addr);

void
vm_map(vm_t linaddr, pym_t pyaddr);

// 文件映射(分配虚拟页面,不会实际分配物理页面)
vm_t
vm_map_file(vm_t addr, size_t length, int fd, off_t offset);

//=========================
// 用户态内存
//=========================
vm_t
vm_alloc_stack();

// 复制页表
int
vm_copy_pagetable(pdt_t cur_pdt, pdt_t new_pdt);

// 复制一个页面
int
vm_fork_page(vm_t addr);

// 分配一个页面
int
vm_alloc_page(vm_t addr);


/*

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

// 分配页表
pt_t
alloc_page_table(pde_t *pde);

// 虚拟内存的最大限制(分配虚拟页面,不会实际分配物理页面)
size_t
vm_user_grow(int sz);


#endif // __MEMORY_H__
