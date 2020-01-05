#include "multiboot.h"
#include "sys/types.h"
#include "asm.h"
#include "arch/page.h"
#include "memory.h"
#include "arch/arch.h"
#include "config.h"
// **************//
//    内存布局    //
// **************//
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
// |-----------| kvm_end + DIRECT_HEAP

extern void start_main();

static vm_t init_page_map();

void boot_trap(uint32_t magic, multiboot_info_t* binfo)
{
    if (magic != MULTIBOOT_HEADER_MAGIC)
        return;
    // 将内核映射到高地址:_VMA
    vm_t pagemap_end = init_page_map();
    // 使用boot的地址用作内核栈
    __asm__ volatile (
        "movl %0, %%esp \n"
        "xorl %%eax, %%eax \n"
        "movl %%eax, %%ebp \n"
        : :"r"(PAGE_CEILING((vm_t)&boot_end))
        : "eax"
    );
    // 设备初始化
    init_isa();
    start_main(pagemap_end);
}

static vm_t init_page_map()
{
    vm_t kernel_size = (vm_t)&kernel_end - (vm_t)&kernel_start;
    vm_t kvm_start   = pym2vm((pym_t)&kernel_start);
    vm_t kvm_end     = PAGE_CEILING(kvm_start + kernel_size);
    volatile pdt_t g_pdt = (pdt_t)vm2pym(kvm_end);
    kvm_end += PAGE_SIZE;

    int cur_pdi = get_pde_index(kvm_start);
    int low_pdi = get_pde_index((vm_t)&kernel_start);
    volatile pt_t cur_pt = (pt_t)vm2pym(kvm_end);
    g_pdt[cur_pdi] = PAGE_ENTRY((vm_t)cur_pt);
    g_pdt[low_pdi] = g_pdt[cur_pdi];  // 映射boot(完成自举)
    kvm_end += PAGE_SIZE;
    
    const size_t heapsize = FIXED_HEAP << 20;

    for (vm_t kaddr = kvm_start; kaddr < kvm_end + heapsize; kaddr+= PAGE_SIZE) {
        int pti = get_pte_index(kaddr);
        if (pti == (PAGE_ENTRY_NUM-1)) {
            cur_pdi = get_pde_index(kaddr) + 1;
            g_pdt[cur_pdi] = PAGE_ENTRY(vm2pym(kvm_end));
            cur_pt[pti] = PAGE_ENTRY(vm2pym(kaddr));
            cur_pt = (pt_t)kvm_end;
            kvm_end += PAGE_SIZE;
        }
        else {
            cur_pt[pti] = PAGE_ENTRY(vm2pym(kaddr));
        }
    }
    load_pdt((pdt_t)g_pdt);
    enable_paging();
    return kvm_end + PAGE_SIZE;
}
