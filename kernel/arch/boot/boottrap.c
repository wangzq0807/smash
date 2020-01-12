#include "multiboot.h"
#include "sys/types.h"
#include "asm.h"
#include "arch/page.h"
#include "memory.h"
#include "arch/arch.h"
#include "config.h"
#include "string.h"
// **************//
//    内存布局    //
// **************//
// | bios      |
// |-----------| kernel_start + _VMA
// | boot.text |
// |-----------|
// | boot.bss  |
// |-----------| boot_end + _VMA
// | .text     |
// |-----------|
// | .data     |
// |-----------|
// | .bss      |
// |-----------|
// | (other)   |
// |-----------| kernel_end + _VMA
// |  pdt      |
// |-----------|
// |  pt       |
// |-----------| kvm_end
// |  heap     |
// |-----------| (kvm_end + FIXED_HEAP) align to 4M

extern char _tmpstack;
extern vm_t   kernel_heap;
extern size_t kernel_heap_size;
extern void start_main();

static void init_page_map();

void boot_trap(uint32_t magic, multiboot_info_t* binfo)
{
    if (magic != MULTIBOOT_HEADER_MAGIC)
        return;
    // 将内核映射到高地址:_VMA
    init_page_map();
    // 使用boot的地址用作内核栈
    size_t *current_task = (size_t*)&kernel_start;
    *current_task = 0;  // 还没有创建任务
    __asm__ volatile (
        "movl %0, %%esp \n"
        "movl %0, %%ebp \n"
        : :"r"(PAGE_CEILING((vm_t)&boot_end))
        : "eax"
    );
    start_main();
}

static void init_page_map()
{
    pym_t cur_end = PAGE_CEILING((pym_t)&kernel_end);
    size_t kernel_size = cur_end;
    // 计算直接映射空间总大小
    const size_t fixedheap = FIXED_HEAP << 20;
    size_t map_size = PAGE_CEILING(kernel_size + fixedheap + PAGE_SIZE); // kernel+heap+gdt
    map_size += PAGE_CEILING(map_size / PAGE_SIZE * PAGE_ENTRY_SIZE);   // + pt
    map_size = PAGE_HUGE_CEILING(map_size);
    // 修改 gdt
    vm_t kvm_start = pym2vm(0);
    int pdi_beg = get_pde_index(kvm_start);
    volatile pdt_t pdt = (pdt_t)cur_end;  // alloc pdt
    cur_end += PAGE_SIZE; // + gdt
    int numpde = (map_size + PAGE_ENTRY_NUM*PAGE_SIZE - 1) / (PAGE_ENTRY_NUM*PAGE_SIZE);
    for (int i = 0; i < numpde; ++i) {
        pdt[i + pdi_beg] = PAGE_ENTRY(cur_end + i*PAGE_SIZE);
    }
    // 映射boot(假定boot)
    pdt[0] = pdt[pdi_beg];
    // 修改pt
    int numpte = map_size / PAGE_SIZE;
    volatile pt_t pt = (pt_t)cur_end;  // alloc pdt
    for (int i = 0; i < numpte; ++i) {
        pt[i] = PAGE_ENTRY(i*PAGE_SIZE);
    }

    load_pdt((pym_t)pdt);
    enable_paging();
    // 传递参数
    pym_t pt_end = cur_end + numpde*PAGE_SIZE;
    kernel_heap = pym2vm(pt_end);
    kernel_heap_size = map_size - pt_end;
}
