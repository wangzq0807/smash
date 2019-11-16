#include "memory.h"
#include "asm.h"
#include "string.h"
#include "sys/types.h"
#include "log.h"

#define PAGE_PRESENT        1
#define PAGE_WRITE          2
#define PAGE_USER           1 << 2
#define PAGE_ACCESSED       1 << 5
#define PAGE_DIRTY          1 << 6

#define PDT     (0)
#define PT1     (PDT + PAGE_SIZE)

static int inline is_page_exist(pte_t pte) {
    return pte & PAGE_PRESENT;
}

void init_memory()
{
    // 开启分页,映射前4M物理内存
    volatile pdt_t pdt = get_pdt();
    memset(pdt, 0, PAGE_SIZE);
    volatile pt_t pt1 = (pt_t)PT1;
    size_t addr = 0;
    pdt[0] = PAGE_FLOOR((uint32_t)pt1) | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
    for (int i = 0; i < PAGE_SIZE/sizeof(pde_t); ++i) {
        pt1[i] = PAGE_FLOOR(addr) | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
        addr += PAGE_SIZE;
    }
    smash_memory();
    load_pdt(pdt);
    enable_paging();
}

void* alloc_page()
{
    // 640K的最后一个页面0x9F000不能用?
    static uint32_t tail = 0x9F000;
    tail -= PAGE_SIZE;
    return (void *)tail;
}

void map_mem(const vm_t src, const vm_t dst, uint32_t bsize)
{
    // KLOG(DEBUG, "map_mem src:%X dst:%X bsize:%d", src, dst, bsize)
    volatile pdt_t pdt = get_pdt();
    const int dst_pdei = get_pde_index(dst);
    const int dst_ptei = get_pte_index(dst);
    int pagenum = (bsize + PAGE_SIZE - 1) >> PAGE_LOG_SIZE;
    for (int i = 0; i < pagenum; ++i)
    {
        const int new_pdei = dst_pdei + (dst_ptei + i) / PAGE_INT_SIZE;
        const int new_ptei = (dst_ptei + i) % PAGE_INT_SIZE;
        // KLOG(DEBUG, "map_mem new_pdei:%d new_ptei:%d", new_pdei, new_ptei)
        if (!is_page_exist(pdt[new_pdei]))
        {
            const uint32_t ptaddr = (uint32_t)alloc_page();
            pdt[new_pdei] = PAGE_FLOOR(ptaddr) | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
        }
        pt_t pt = (pt_t)PAGE_FLOOR(pdt[new_pdei]);
        pt[new_ptei] = PAGE_FLOOR(src + (i << PAGE_LOG_SIZE)) | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
        invlpg(dst + (i << PAGE_LOG_SIZE));
    }
    smash_memory();
}

