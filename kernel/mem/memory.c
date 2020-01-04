#include "memory.h"
#include "lib/log.h"
#include "lib/list.h"
#include "arch/page.h"
#include "arch/task.h"
#include "asm.h"
#include "pymem.h"

// 用户内存
const uint32_t usr_vm_beg = 0;
const uint32_t usr_vm_end = 0xFE000000;
// 内核代码段内存
const uint32_t knl_code_beg = 0xFE000000;
// 物理地址与虚拟地址间的差值,内核段的开始位置
uint32_t    knl_space_begin   = 0;

void
_init_vm()
{
    KLOG(DEBUG, "kernel_start %X, kernel_end %X", &kernel_start, &kernel_end);
    if (usr_vm_end > knl_code_beg)
        KLOG(ERROR, "usr_vm_end overflow!");
    if (usr_vm_beg & ((4<<20) - 1))
        KLOG(ERROR, "usr_vm_beg not align 4M!");
    if (knl_code_beg & ((4<<20) - 1))
        KLOG(ERROR, "knl_code_beg not align 4M!");

    // 计算vm_offset
    size_t cr3 = get_cr3();
    const int page_index = cr3 >> PAGE_SHIFT;
    int npti = page_index & (PAGE_ENTRY_NUM-1);
    int npdi = page_index >> 10;
    pdt_t pdt = (pdt_t)cr3;
    pt_t pt = (pt_t)PAGE_FLOOR(pdt[npdi]);
    pte_t pte = pt[npti];
    
    size_t pyaddr = PAGE_FLOOR(pte);
    knl_space_begin = knl_code_beg - pyaddr;
    KLOG(DEBUG, "knl_space_begin: %X", knl_space_begin);
    // 映射内核空间地址

    // 0 - 1M 映射到高位地址
    for (int i = 0; i < (1<<20); i+=PAGE_SIZE)
    {
        if (!is_pypage_used(i))
            continue;
        vm_t vaddr = paddr2vaddr(i);
        npdi = get_pde_index(vaddr);
        //pdt[npdi]
        //npti = get_pte_index(vaddr);
        pt[npti] = i | PAGE_WRITE | PAGE_USER | PAGE_PRESENT;
    }
    //======================================================
    move_esp(knl_space_begin);
    pdt = get_pdt();
    int pde_end = get_pde_index(knl_space_begin);
    for (int i = 0; i < pde_end; i++)
        pdt[i] = 0;
    pt = pde2pt(pdt[pde_end]);
    int pte_end = get_pte_index(knl_space_begin);
    for (int i = 0; i < pte_end; i++)
        pt[i] = 0;
}

void
vm_init()
{
    init_pymemory();
    // knl_code_beg - END : 存放内核代码(一一映射)
    _init_vm();
}

void*
vm_kalloc()
{
    pdt_t pdt = get_pdt();
    int pde_beg = get_pde_index(knl_code_beg);
    int pde_end = PAGE_ENTRY_NUM;
    for (int pdei = pde_beg; pdei < pde_end; ++pdei) 
    {
        if (!is_page_exist(pdt[pdei]))
            KLOG(ERROR, "PDE is not exist!");
        pt_t pt = pde2pt(pdt[pdei]);
        for (int ptei = 0; ptei < PAGE_ENTRY_NUM; ++ptei)
        {
            if (!is_page_exist(pt[ptei]))
            {
                vm_t vaddr = make_vaddr(pdei, ptei);
                size_t paddr = vaddr2paddr(vaddr);
                pt[ptei] = paddr | PAGE_WRITE | PAGE_USER | PAGE_PRESENT;
                return (void*)vaddr;
            }
        }
    }
    return 0;
}

vm_t
mem_ualloc_page()
{
    int pde_beg = get_pde_index(usr_vm_beg);
    int pde_end = get_pde_index(usr_vm_end);
    for (int pdei = pde_beg; pdei < pde_end; ++pdei)
    {

    }

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
    for (int i = 0; i < node_num; i++ ) {
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

void
pypage_copy(uint32_t pydst, uint32_t pysrc, size_t num)
{
    map_vm_page(0xFFFFF000, pydst);
    map_vm_page(0xFFFFE000, pysrc);

    int *dist_page = (int *)0xFFFFF000;
    const int *src_page = (const int *)0xFFFFE000;
    size_t len = num * PAGE_ENTRY_NUM;
    while (--len) {
        *dist_page++ = *src_page++;
    }
    unmap_vm_page(0xFFFFF000);
    unmap_vm_page(0xFFFFE000);
}
*/

vm_t
alloc_vm_page()
{
    pdt_t pdt = get_pdt();
    pt_t cur_pte = pde2pt(pdt[0]);
    for (int npte = 256; npte < PAGE_ENTRY_NUM; ++npte) {
        if ( (cur_pte[npte] & PAGE_PRESENT) == 0) {
            uint32_t pyaddr = (npte << 12);
            cur_pte[npte] = PAGE_FLOOR(pyaddr) | PAGE_WRITE | PAGE_USER | PAGE_PRESENT;
            vm_t vmret = (vm_t)(pyaddr); // 物理地址与虚拟地址相同
            invlpg(vmret);
            int *words = (int *)vmret;
            KLOG(DEBUG, "%s 0x%x", __FUNCTION__, vmret);
            // 页面清0
            for (int i = 0; i < PAGE_ENTRY_NUM; ++i)
                words[i] = 0;
            return vmret;
        }
    }
    return NULL;
}

void
release_vm_page(vm_t addr)
{
    uint32_t linear = (uint32_t)addr;
    pt_t pt = get_pt(linear);
    uint32_t npte = get_pte_index(linear);
    pt[npte] = pt[npte] & ~PAGE_PRESENT;
    invlpg(addr);
    KLOG(DEBUG, "%s 0x%x", __FUNCTION__, addr);
}

void
map_vm_page(vm_t linaddr, uint32_t pyaddr)
{
    pdt_t pdt = get_pdt();
    uint32_t npde = get_pde_index(linaddr);
    if ((pdt[npde] & PAGE_PRESENT) == 0) {
        int peaddr = (int)alloc_vm_page();
        pdt[npde] = PAGE_FLOOR(peaddr) | PAGE_PRESENT | PAGE_USER | PAGE_WRITE;
        invlpg(peaddr);
    }
    pt_t pt = pde2pt(pdt[npde]);
    uint32_t npte = get_pte_index(linaddr);
    pt[npte] = PAGE_FLOOR(pyaddr) | PAGE_PRESENT | PAGE_USER | PAGE_WRITE;
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
        new_pte = (pt_t)alloc_vm_page();
    }
    new_pdt[0] = PAGE_FLOOR((uint32_t)new_pte) | PAGE_WRITE | PAGE_USER | PAGE_PRESENT;
    for (int npte = 0; npte < PAGE_ENTRY_NUM; ++npte) {
        new_pte[npte] = cur_pte[npte];
    }
}

pt_t
alloc_page_table(pde_t *pde)
{
    vm_t addr = alloc_vm_page();
    *pde = PAGE_FLOOR((uint32_t)addr) | PAGE_WRITE | PAGE_USER | PAGE_PRESENT;
    return (pt_t)addr;
}

uint32_t
alloc_spage() {
    // 640K的最后一个页面0x9F000不能用?
    static uint32_t tail = 0x9F000;
    tail -= PAGE_SIZE;
    return tail;
}

vm_t
mm_vfile(vm_t addr, size_t length, int fd, off_t offset)
{
    if (PAGE_MARK(addr) > 0)
        return 0;
    pdt_t pdt = get_pdt();
    int npde = get_pde_index(addr);
    if ((pdt[npde] & PAGE_PRESENT) == 0)
        alloc_page_table(&pdt[npde]);
    pt_t pt = pde2pt(pdt[npde]);
    int npte = get_pte_index(addr);
    if (pt[npte] & PAGE_PRESENT)
        return 0;
    // 所需映射的总页面数
    const int npage = (length + PAGE_SIZE - 1) >> PAGE_SHIFT;
    // TODO: 失败后,要回收已映射的页表项
    // 当前页表的映射 pte
    int lessnum = PAGE_ENTRY_NUM - npte;
    lessnum = npage > lessnum ? lessnum : npage;
    for (int n = npte; n < npte + lessnum; ++n)
    {
        pt[n] = PAGE_FLOOR(offset) | (fd << 1);
        offset += PAGE_SIZE;
    }
    if (npage <= lessnum)
        return addr;
    npde++;
    // 页目录表的映射
    const int pdenum = npde + (npage - lessnum) / PAGE_ENTRY_NUM;
    for (int n = npde; n < pdenum; ++n)
    {
        if ((pdt[n] & PAGE_PRESENT) == 0)
            alloc_page_table(&pdt[n]);

        pt = pde2pt(pdt[n]);
        for (int nn = 0; nn < PAGE_ENTRY_NUM; ++nn)
        {
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
    for (int n = 0; n < morenum; ++n)
    {
        pt[n] = PAGE_FLOOR(offset) | (fd << 1);
        offset += PAGE_SIZE;
    }

    return addr;
}

size_t
grow_user_vm(int sz)
{
    Task *ts = current_task();
    size_t oldsz = ts->ts_size;
    ts->ts_size += sz;
    return oldsz;
}
