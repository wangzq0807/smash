#ifndef __LINEAR_H__
#define __LINEAR_H__

#include "asm.h"
#include "sys/types.h"

#define PAGE_LOG_SIZE   12
#define PAGE_SIZE     (1 << PAGE_LOG_SIZE)
#define PAGE_INT_SIZE         (PAGE_SIZE/sizeof(int))
#define PAGE_CEILING(addr)    (((addr) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define PAGE_FLOOR(addr)      ((addr) & ~(PAGE_SIZE - 1))
#define PAGE_MARK(addr)         ((addr) & (PAGE_SIZE - 1))
//====================================
//  常用线性地址操作
//====================================
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
    return (pt_t)PAGE_FLOOR(pde);
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
    return (pt_t)PAGE_FLOOR(pde);
}

static inline uint32_t
pte2pypage(pte_t pte) {
    return PAGE_FLOOR(pte); 
}

#endif // __LINEAR_H__