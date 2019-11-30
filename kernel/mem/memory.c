#include "memory.h"
#include "lib/log.h"
#include "lib/list.h"
#include "arch/page.h"
#include "arch/task.h"
#include "asm.h"
#include "pymem.h"
/*
typedef struct _PageNode PageNode;
struct _PageNode {
    uint32_t    pn_page : 20,
                pn_refs : 12;
    PageNode    *pn_next;
    ListEntity  pn_hash_link;
};

typedef struct _PageListHead PageListHead;
struct _PageListHead {
    uint32_t    pl_dummyLock;   // fixme: 链表加锁
    PageNode    *pl_free;
};

static PageListHead free_memory;
static int cur_start;
static int max_end;

#define BUFFER_HASH_LEN 1024
#define HASH_MAGIC   (BUFFER_HASH_LEN * 1000 / 618)
#define HASH(val)    ((val)*HASH_MAGIC % BUFFER_HASH_LEN)

ListHead    hash_map[BUFFER_HASH_LEN];

static error_t
_remove_hash_entity(PageNode *node)
{
    uint32_t hash_val = HASH(node->pn_page);
    ListHead *head = &hash_map[hash_val];
    list_remove_entity(head, &node->pn_hash_link);

    return 0;
}

static PageNode *
_get_hash_entity(uint32_t page)
{
    uint32_t hash_val = HASH(page);
    ListHead head = hash_map[hash_val];
    ListEntity *begin = list_get_head(&head);
    ListEntity *iter = begin;
    while (iter != NULL) {
        PageNode *node = TO_INSTANCE(iter, PageNode, pn_hash_link);
        if (node != NULL && node->pn_page == page)
            return node;
        iter = iter->le_next;
        if (iter == begin)
            break;
    }
    return NULL;
}

static error_t
_put_hash_entity(PageNode *node)
{
    PageNode *org = _get_hash_entity(node->pn_page);
    if (org != NULL)
        _remove_hash_entity(org);

    uint32_t hash_val = HASH(node->pn_page);
    ListHead *head = &hash_map[hash_val];
    list_push_front(head, &node->pn_hash_link);

    return 0;
}
    if ( (start >= end)
        || (size_t)(start) & (PAGE_SIZE - 1)
        || (size_t)(end) & (PAGE_SIZE - 1) ) {
        KLOG(ERROR, "wrong memory range\n");
        return;
    }

    max_end = end;
    cur_start = start;
    free_memory.pl_free = NULL;
*/
// 一一映射内存
uint32_t const_vm = 0;
uint32_t const_vm_end = 0;
// 用户内存
uint32_t usr_vm_beg = 0;
uint32_t usr_vm_end = 0;
// 内核内存
uint32_t knl_vm_beg = 0;
uint32_t knl_vm_end = 0;

void
init_vm(uint32_t cvmend, uint32_t usrvmbeg, uint32_t knlvmbeg)
{
    // 虚拟内存边界要对齐到4M
    if (cvmend & ~((1<<20) - 1))
        KLOG(ERROR, "const_vm_end not align 4M!");
    if (usrvmbeg & ~((1<<20) - 1))
        KLOG(ERROR, "usr_vm_beg not align 4M!");
    if (knlvmbeg & ~((1<<20) - 1))
        KLOG(ERROR, "knl_vm_beg not align 4M!");
    const_vm_end = cvmend;
    usr_vm_beg = usrvmbeg;
    usr_vm_end = knl_vm_beg = knlvmbeg;
    knl_vm_end = 0xFFFFFFFF;

    pdt_t pdt = get_pdt();
    pt_t pt0 = pde2pt(pdt[0]);
    // 只保留前3个映射,用于1栈和2页表
    for (int ipte = 3; ipte < PAGE_INT_SIZE; ++ipte)
    {
        pt0[ipte] = 0;
    }
    // 只保留第1个页目录项(映射页表),最后4个页目录项(映射内核代码)
    for (int ipde = 1; ipde < PAGE_INT_SIZE - 4; ++ipde)
    {
        pdt[ipde] = 0;
    }
    load_pdt(pdt);
}

void
init_memory()
{
    init_pymemory();
    // 1页内核栈和2页页表
    alloc_pyrange(0x0, 3*PAGE_SIZE);
    // 0xA0000 - 1M : BIOS
    alloc_pyrange(0xA0000, 0x100000);
    // 1M -2M : 内核代码
    alloc_pyrange(1 << 20, 1 << 20);

    // 0 - 0x400000 : 一一映射,存放页表
    // 0xFE000000 - END : 存放内核代码
    init_vm(4 << 20, 4 << 20, 0xFE000000);
}

vm_t
alloc_pagetable()
{
    pdt_t pdt = get_pdt();
    int pde_beg = get_pde_index(const_vm);
    int pde_end = get_pde_index(const_vm_end);
    for (int pdei = pde_beg; pdei < pde_end; ++pdei)
    {
        if (!is_page_exist(pdt[pdei]))
        {
            KLOG(ERROR, "PDE is not exist!");
        }
        pt_t pt = pde2pt(pdt[pdei]);
        for (int ptei = 0; ptei < PAGE_INT_SIZE; ++ptei)
        {
            if (!is_page_exist(pt[ptei]))
            {
                vm_t vaddr = make_vmaddr(pdei, ptei);
                pt[ptei] = vaddr | PAGE_WRITE | PAGE_USER | PAGE_PRESENT;
                return vaddr;
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
    node_num = MIN(node_num, (max_end - addr) >> PAGE_LOG_SIZE);
    for (int i = 0; i < node_num; i++ ) {
        node_list[i].pn_page = (addr >> PAGE_LOG_SIZE) + i;
        node_list[i].pn_refs = 0;
        node_list[i].pn_next = &node_list[i+1];
    }
    node_list[node_num - 1].pn_next = NULL;

    free_memory.pl_free = &node_list[0];
    cur_start = addr + (node_num << PAGE_LOG_SIZE);
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
        return (ret->pn_page << PAGE_LOG_SIZE);
    }
    else {
        return NULL;
    }
}

int
get_pypage_refs(uint32_t page)
{
    PageNode *node = _get_hash_entity(page >> PAGE_LOG_SIZE);
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

    PageNode *node = _get_hash_entity(page >> PAGE_LOG_SIZE);
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

    PageNode *node = _get_hash_entity(page >> PAGE_LOG_SIZE);
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
    size_t len = num * PAGE_INT_SIZE;
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
    for (int npte = 256; npte < PAGE_INT_SIZE; ++npte) {
        if ( (cur_pte[npte] & PAGE_PRESENT) == 0) {
            uint32_t pyaddr = (npte << 12);
            cur_pte[npte] = PAGE_FLOOR(pyaddr) | PAGE_WRITE | PAGE_USER | PAGE_PRESENT;
            vm_t vmret = (vm_t)(pyaddr); // 物理地址与虚拟地址相同
            invlpg(vmret);
            int *words = (int *)vmret;
            KLOG(DEBUG, "%s 0x%x", __FUNCTION__, vmret);
            // 页面清0
            for (int i = 0; i < PAGE_INT_SIZE; ++i)
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
    for (int npte = 0; npte < PAGE_INT_SIZE; ++npte) {
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
    const int npage = (length + PAGE_SIZE - 1) >> PAGE_LOG_SIZE;
    // TODO: 失败后,要回收已映射的页表项
    // 当前页表的映射 pte
    int lessnum = PAGE_INT_SIZE - npte;
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
    const int pdenum = npde + (npage - lessnum) / PAGE_INT_SIZE;
    for (int n = npde; n < pdenum; ++n)
    {
        if ((pdt[n] & PAGE_PRESENT) == 0)
            alloc_page_table(&pdt[n]);

        pt = pde2pt(pdt[n]);
        for (int nn = 0; nn < PAGE_INT_SIZE; ++nn)
        {
            pt[nn] = PAGE_FLOOR(offset) | (fd << 1);
            offset += PAGE_SIZE;
        }
    }
    npde += pdenum;
    // 剩余页表的映射 pte
    const int morenum = npage - lessnum - (pdenum*PAGE_INT_SIZE);
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
