#include "multiboot.h"
#include "sys/types.h"
#include "asm.h"
#include "arch/page.h"
#include "mem/linear.h"

static pde_t g_pdt[PAGE_INT_SIZE] __attribute__((aligned(PAGE_SIZE)));

extern void start_main();
extern char _LMA;
extern char _VMA;
extern char kernel_start;   // LMA
extern char kernel_end;     // LMA

void init_page_map();

void boot_trap(uint32_t magic, multiboot_info_t* binfo)
{
    if (magic != MULTIBOOT_HEADER_MAGIC)
        return;
    init_page_map();
    start_main();
}

void init_page_map()
{
    vm_t kernel_size = (vm_t)&kernel_end - (vm_t)&kernel_start;
    vm_t kvm_start   = (vm_t)&_VMA;
    vm_t kvm_end     = PAGE_CEILING(kvm_start+ kernel_size);
    vm_t kvm_offset  = kvm_start - ((vm_t)&_LMA);   // 虚拟地址和物理地址的差值

    int  cur_pdi = get_pde_index(kvm_start);
    volatile pt_t cur_pt = (pt_t)(kvm_end - kvm_offset);
    g_pdt[cur_pdi] = PAGE_FLOOR(kvm_end - kvm_offset) | PAGE_PRESENT | PAGE_WRITE;
    kvm_end += PAGE_SIZE;

    for (vm_t kaddr = kvm_start; kaddr < kvm_end; kaddr+= PAGE_SIZE) {
        int pti = get_pte_index(kaddr);
        if (pti == (PAGE_INT_SIZE-1)) {
            cur_pdi = get_pde_index(kaddr) + 1;
            g_pdt[cur_pdi] = PAGE_FLOOR(kvm_end - kvm_offset) | PAGE_PRESENT | PAGE_WRITE;
            cur_pt[pti] = PAGE_FLOOR(kaddr - kvm_offset) | PAGE_PRESENT | PAGE_WRITE;
            cur_pt = (pt_t)kvm_end;
            kvm_end += PAGE_SIZE;
        }
        else {
            cur_pt[pti] = PAGE_FLOOR(kaddr - kvm_offset) | PAGE_PRESENT | PAGE_WRITE;
        }
    }
    load_pdt(g_pdt);
    enable_paging();
}
