#ifndef __LINEAR_H__
#define __LINEAR_H__

#include "asm.h"
#include "sys/types.h"
#include "arch/page.h"

extern char _VMA;

static inline vm_t pym2vm(pym_t paddr) {
    return paddr + (vm_t)&_VMA;
}

static inline pym_t vm2pym(vm_t vaddr) {
    return vaddr - (vm_t)&_VMA;;
}
//====================================
// 常用线性地址操作
//====================================
// static inline pdt_t to_pdt(pm_t addr) {
//     pdt_t pdt;
//     pdt.pde_ptr = (pde_t *)pm2vm(addr);;
//     return pdt;
// }

static inline pdt_t get_pdt() {
    size_t cr3 = get_cr3();
    return (pdt_t)pym2vm(cr3);
}

static inline int
get_pde_index(vm_t linear) {
    return linear >> 22;
}

static inline int
get_pte_index(vm_t linear) {
    return (linear >> 12) & 0x3FF;
}

static inline pde_t
get_pde(vm_t linear) {
    uint32_t npde = linear >> 22;
    pdt_t pdt = (pdt_t)get_pdt();
    return pdt[npde];
}

static inline pt_t
get_pt(vm_t linear) {
    pde_t pde = get_pde(linear);
    return (pt_t)pym2vm(PAGE_FLOOR(pde));
}

static inline pte_t
get_pte(vm_t linear) {
    uint32_t npte = (linear >> 12) & 0x3FF;
    pt_t pt = get_pt(linear);
    return pt[npte];
}

static inline uint32_t
get_pypage(vm_t linear) {
    pte_t pte = get_pte(linear);
    return PAGE_FLOOR(pte);
}

static inline pt_t
pde2pt(pde_t pde) {
    return (pt_t)pym2vm(PAGE_FLOOR(pde));
}

static inline uint32_t
pte2pypage(pte_t pte) {
    return PAGE_FLOOR(pte); 
}

static inline int is_page_exist(pte_t pte) {
    return pte & PAGE_PRESENT;
}

static inline vm_t make_vaddr(int pdei, int ptei) {
    return (pdei<<22) + (ptei<<12);
}

#endif // __LINEAR_H__