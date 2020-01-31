#include "memory.h"
#include "lib/log.h"
#include "lib/list.h"
#include "arch/page.h"
#include "arch/task.h"
#include "asm.h"
#include "mem/frame.h"
#include "string.h"

vm_t   kernel_heap_beg = 0;
size_t kernel_heap_size = 0;

void
_vm_init()
{
    KLOG(DEBUG, "heap 0x%x, size 0x%x", kernel_heap_beg, kernel_heap_size);
    // 清除boot的映射
    // pdt_t pdt = get_pdt();
    // pdt[0] = 0;
}

void
memory_setup()
{
    frame_init();
    // knl_code_beg - END : 存放内核代码(一一映射)
    _vm_init();
}

void*
vm_alloc()
{
    pym_t paddr = frame_alloc();
    vm_t vaddr = pym2vm(paddr);
    KLOG(DEBUG, "vm_alloc 0x%x", vaddr);
    memset((void*)vaddr, 0, PAGE_SIZE);
    return (void*)vaddr;
    // pdt_t pdt = get_pdt();
    // pt_t cur_pte = pde2pt(pdt[0]);
    // for (int npte = 256; npte < PAGE_ENTRY_NUM; ++npte) {
    //     if ((cur_pte[npte] & PAGE_PRESENT) == 0) {
    //         uint32_t pyaddr = (npte << 12);
    //         cur_pte[npte] = PAGE_FLOOR(pyaddr) | PAGE_WRITE | PAGE_USER | PAGE_PRESENT;
    //         vm_t vmret = (vm_t)(pyaddr); // 物理地址与虚拟地址相同
    //         invlpg(vmret);
    //         int *words = (int *)vmret;
    //         KLOG(DEBUG, "%s 0x%x", __FUNCTION__, vmret);
    //         // 页面清0
    //         for (int i = 0; i < PAGE_ENTRY_NUM; ++i)
    //             words[i] = 0;
    //         return vmret;
    //     }
    // }
}

int
vm_free(void* addr)
{
    pym_t paddr = vm2pym((vm_t)addr);
    frame_release(paddr);
    return 0;
    // uint32_t linear = (uint32_t)addr;
    // pt_t pt = get_pt(linear);
    // uint32_t npte = get_pte_index(linear);
    // pt[npte] = pt[npte] & ~PAGE_PRESENT;
    // invlpg(addr);
    // KLOG(DEBUG, "%s 0x%x", __FUNCTION__, addr);
}

vm_t
vm_alloc_stack()
{
    KLOG(DEBUG, "vm_alloc_stack");
    pym_t paddr = frame_alloc();
    vm_map((vm_t)(&_VMA - PAGE_SIZE), paddr);
    return (vm_t)(&_VMA - PAGE_SIZE);
}

int
vm_copy_pagetable(pdt_t cur_pdt, pdt_t new_pdt)
{
    // 复制页表(NOTE:不要修改内核页表)
    int knl_pde_begin = ((vm_t)&_VMA / PAGE_HUGE_SIZE);
    for (int npde = 0; npde < knl_pde_begin; ++npde) {
        if (cur_pdt[npde] & PAGE_PRESENT) {
            pt_t cur_pt = pde2pt(cur_pdt[npde]);
            pt_t new_pt = alloc_page_table(&new_pdt[npde]);
            for (int npte = 0; npte < PAGE_ENTRY_NUM; ++npte) {
                if (cur_pt[npte] & PAGE_PRESENT) {
                    cur_pt[npte] &= ~PAGE_WRITE;
                    new_pt[npte] = cur_pt[npte];
                    frame_add_ref(PAGE_FLOOR(cur_pt[npte]));
                }
            }
        }
    }
    // 复制内核的页目录
    for (int npde = knl_pde_begin; npde < PAGE_ENTRY_NUM; ++npde) {
        new_pdt[npde] = cur_pdt[npde];
    }
    return 0;
}

int
vm_fork_page(vm_t addr)
{
    pt_t pt = get_pt(addr);
    int npte = get_pte_index(addr);
    int nref = frame_get_ref(PAGE_FLOOR(pt[npte]));
    KLOG(DEBUG, "fork_page addr:0x%x ref:%d", addr, nref);
    if (nref == 1) {
        pt[npte] |= PAGE_WRITE;
        return 0;
    }
    frame_release(PAGE_FLOOR(pt[npte]));

    pym_t paddr = frame_alloc();
    vm_t vaddr = pym2vm(paddr);
    memcpy((void*)vaddr, (void*)addr, PAGE_SIZE);
    vm_map(addr, paddr);
    // //uint32_t pyaddr = pte2pypage(pt[npte]);
    // int refs = 0;//get_pypage_refs(pyaddr);
    // if (refs > 1) {
    //     /*
    //     uint32_t new_page = alloc_pypage();
    //     pypage_copy(new_page, pyaddr, 1);
    //     release_pypage(pyaddr); // 减引用计数

    //     pt[npte] = PAGE_FLOOR((uint32_t)new_page) | PAGE_PRESENT | PAGE_USER | PAGE_WRITE;
    //     */
    // }
    // else {
    //     pt[npte] |= PAGE_WRITE;
    // }
    return 0;
}

int
vm_alloc_page(vm_t addr)
{
    pde_t pde = get_pde(addr);
    if ((pde & PAGE_PRESENT) == 0) {
        KLOG(ERROR, "addr:0x%x is invalid!", addr);
        return -1;
    }
    pym_t pyaddr = frame_alloc();
    vm_map(addr, pyaddr);
    return 0;
}

vm_t
mem_ualloc_page()
{
    // int pde_beg = get_pde_index(usr_vm_beg);
    // int pde_end = get_pde_index(usr_vm_end);
    // for (int pdei = pde_beg; pdei < pde_end; ++pdei)
    // {

    // }

    return 0;
}

vm_t
mem_kalloc_page()
{
    return 0;
}

int
mem_release_page(vm_t vaddr)
{
    return 0;
}

/*
static void
_new_freelist(uint32_t addr)
{
    // 一个页面用来建立空闲页的链表(大约会新增1M的可用内存)
    PageNode *node_list = (PageNode*)alloc_spage();
    int node_num = PAGE_SIZE / sizeof(PageNode);
    node_num = MIN(node_num, (max_end - addr) >> PAGE_SHIFT);
    for (int i = 0; i < node_num; i++) {
        node_list[i].pn_page = (addr >> PAGE_SHIFT) + i;
        node_list[i].pn_refs = 0;
        node_list[i].pn_next = &node_list[i+1];
    }
    node_list[node_num - 1].pn_next = NULL;

    free_memory.pl_free = &node_list[0];
    cur_start = addr + (node_num << PAGE_SHIFT);
}

uint32_t
alloc_pypage()
{
    // 返回链表中第一个空闲内存页，同时把头指针指向下一个空闲内存页
    PageNode *ret = free_memory.pl_free;
    if (ret != NULL) {
        free_memory.pl_free = ret->pn_next;
    }
    else if (cur_start < max_end) {
        _new_freelist(cur_start);
        ret = free_memory.pl_free;
        free_memory.pl_free = ret->pn_next;
    }

    if (ret != NULL) {
        ret->pn_refs = 1;
        _put_hash_entity(ret);
        return (ret->pn_page << PAGE_SHIFT);
    }
    else {
        return NULL;
    }
}

int
get_pypage_refs(uint32_t page)
{
    PageNode *node = _get_hash_entity(page >> PAGE_SHIFT);
    if (node != NULL)
        return node->pn_refs;
    else
        return 0;
}

int
add_pypage_refs(uint32_t page)
{
    if ((size_t)(page) & (PAGE_SIZE - 1)) {
        KLOG(ERROR, "wrong page address\n");
        return -1;
    }

    PageNode *node = _get_hash_entity(page >> PAGE_SHIFT);
    if (node != NULL) {
        node->pn_refs++;
        return node->pn_refs;
    }
    else {
        return -1;
    }
}

int
release_pypage(uint32_t page)
{
    if ((size_t)(page) & (PAGE_SIZE - 1)) {
        KLOG(ERROR, "wrong page address");
        return -1;
    }

    PageNode *node = _get_hash_entity(page >> PAGE_SHIFT);
    if (node == NULL) {
        KLOG(ERROR, "page %x is not found", page);
        return -1;
    }

    if (node->pn_refs == 1) {
        node->pn_refs = 0;
        _remove_hash_entity(node);
        // 将新释放的内存页链接到链表头部
        node->pn_next = free_memory.pl_free;
        free_memory.pl_free = node;
    }
    else {
        node->pn_refs--;
    }
    return node->pn_refs;
}

uint32_t
get_free_space()
{
    PageNode *node = free_memory.pl_free;
    uint32_t cnt = 0;
    while (node != NULL) {
        cnt++;
        node = node->pn_next;
    }
    return cnt;
}

*/


void
vm_map(vm_t linaddr, uint32_t pyaddr)
{
    pdt_t pdt = get_pdt();
    uint32_t npde = get_pde_index(linaddr);
    if ((pdt[npde] & PAGE_PRESENT) == 0) {
        vm_t peaddr = (vm_t)vm_alloc();
        pdt[npde] = PAGE_ENTRY(vm2pym(peaddr));
        invlpg(peaddr);
    }
    pt_t pt = pde2pt(pdt[npde]);
    uint32_t npte = get_pte_index(linaddr);
    pt[npte] = PAGE_ENTRY(pyaddr);
    
    invlpg(linaddr);
}

void
unmap_vm_page(vm_t linaddr)
{
    pdt_t pdt = get_pdt();
    int npde = get_pde_index(linaddr);
    if (pdt[npde] & PAGE_PRESENT) {
        pt_t pt = pde2pt(pdt[npde]);
        uint32_t npte = get_pte_index(linaddr);
        pt[npte] = 0;
        invlpg(linaddr);
    }
}

void
switch_vm_page(pdt_t cur_pdt, pdt_t new_pdt)
{
    // 复制内核所在的0 - 4M页表
    pt_t cur_pte = pde2pt(cur_pdt[0]);
    pt_t new_pte = NULL;

    if (new_pdt[0] & PAGE_PRESENT)
        new_pte = pde2pt(new_pdt[0]);
    else {
        new_pte = (pt_t)vm_alloc();
    }
    new_pdt[0] = PAGE_FLOOR((uint32_t)new_pte) | PAGE_WRITE | PAGE_USER | PAGE_PRESENT;
    for (int npte = 0; npte < PAGE_ENTRY_NUM; ++npte) {
        new_pte[npte] = cur_pte[npte];
    }
}

pt_t
alloc_page_table(pde_t *pde)
{
    KLOG(DEBUG, "alloc_page_table");
    pym_t paddr = frame_alloc();
    *pde = PAGE_ENTRY(paddr);
    pt_t pt = pde2pt(*pde);
    memset(pt, 0, PAGE_SIZE);
    return pt;
}

vm_t
vm_map_file(vm_t addr, size_t length, int fd, off_t offset)
{
    KLOG(DEBUG, "vm_map_file addr:0x%x len:%d file:%d off:%d", addr, length, fd, offset);
    if (PAGE_MARK(addr) > 0)
        return 0;
    pdt_t pdt = get_pdt();
    int npde = get_pde_index(addr);
    if ((pdt[npde] & PAGE_PRESENT) == 0)
        alloc_page_table(&pdt[npde]);
    pt_t pt = pde2pt(pdt[npde]);
    int npte = get_pte_index(addr);
    if (pt[npte] & PAGE_PRESENT) {
        KLOG(ERROR, "vm_map_file addr:%x is used!!", addr);
        return 0;
    }
    // 所需映射的总页面数
    const int npage = PAGE_CEILING(length) >> PAGE_SHIFT;
    // TODO: 失败后,要回收已映射的页表项
    // 当前页表的映射 pte
    int lessnum = PAGE_ENTRY_NUM - npte;
    lessnum = npage > lessnum ? lessnum : npage;
    for (int n = npte; n < npte + lessnum; ++n) {
        pt[n] = PAGE_FLOOR(offset) | (fd << 1);
        offset += PAGE_SIZE;
    }
    if (npage <= lessnum)
        return addr;
    npde++;
    // 页目录表的映射
    const int pdenum = npde + (npage - lessnum) / PAGE_ENTRY_NUM;
    for (int n = npde; n < pdenum; ++n) {
        if ((pdt[n] & PAGE_PRESENT) == 0)
            alloc_page_table(&pdt[n]);
        pt = pde2pt(pdt[n]);
        for (int nn = 0; nn < PAGE_ENTRY_NUM; ++nn) {
            pt[nn] = PAGE_FLOOR(offset) | (fd << 1);
            offset += PAGE_SIZE;
        }
    }
    npde += pdenum;
    // 剩余页表的映射 pte
    const int morenum = npage - lessnum - (pdenum*PAGE_ENTRY_NUM);
    if ((pdt[npde] & PAGE_PRESENT) == 0)
        alloc_page_table(&pdt[npde]);
    pt = pde2pt(pdt[npde]);
    for (int n = 0; n < morenum; ++n) {
        pt[n] = PAGE_FLOOR(offset) | (fd << 1);
        offset += PAGE_SIZE;
    }

    return addr;
}

size_t
vm_user_grow(int sz)
{
    Task *ts = current_task();
    size_t oldsz = ts->ts_size;
    ts->ts_size += sz;
    return oldsz;
}
