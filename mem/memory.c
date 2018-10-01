#include "memory.h"
#include "log.h"
#include "list.h"
#include "arch/page.h"
#include "asm.h"

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
    remove_entity(head, &node->pn_hash_link);

    return 0;
}

static PageNode *
_get_hash_entity(uint32_t page)
{
    uint32_t hash_val = HASH(page);
    ListHead head = hash_map[hash_val];
    ListEntity *iter = head.lh_list;
    while (iter != NULL) {
        PageNode *node = TO_INSTANCE(iter, PageNode, pn_hash_link);
        if (node != NULL && node->pn_page == page)
            return node;
        iter = iter->le_next;
        if (iter == head.lh_list)
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
    push_front(head, &node->pn_hash_link);

    return 0;
}

void
init_memory(uint32_t start, uint32_t end)
{
    if ( (start > end)
        || (size_t)(start) & (PAGE_SIZE - 1)
        || (size_t)(end) & (PAGE_SIZE - 1) ) {
        printk("wrong memory range\n");
        return;
    }

    max_end = end;
    cur_start = start;
    free_memory.pl_free = NULL;
}

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
    /* 返回链表中第一个空闲内存页，同时把头指针指向下一个空闲内存页 */
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
        printk("wrong page address\n");
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
        printk("wrong page address\n");
        return -1;
    }

    PageNode *node = _get_hash_entity(page >> PAGE_LOG_SIZE);
    if (node == NULL) {
        printk("page %x is not found\n", page);
        return -1;
    }

    if (node->pn_refs == 1) {
        node->pn_refs = 0;
        _remove_hash_entity(node);
        /* 将新释放的内存页链接到链表头部 */
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
    size_t len = num * PAGE_SIZE / sizeof(int);
    while (--len) {
        *dist_page++ = *src_page++;
    }
    unmap_vm_page(0xFFFFF000);
    unmap_vm_page(0xFFFFE000);
}

void *
alloc_vm_page()
{
    pde_t *pdt = (pde_t *)get_cr3();
    pte_t *cur_pte = (pte_t *)(pdt[0] & 0xFFFFF000);
    for (int npte = 256; npte < (PAGE_SIZE / sizeof(pte_t)); ++npte) {
        if ( (cur_pte[npte] & PAGE_PRESENT) == 0) {
            uint32_t pyaddr = (npte << 12);
            cur_pte[npte] = PAGE_FLOOR(pyaddr) | PAGE_WRITE | PAGE_USER | PAGE_PRESENT;
            void *ret = (void *)(pyaddr);
            invlpg(ret);
            return ret;
        }
    }
    return NULL;
}

void
release_vm_page(void *addr)
{
    uint32_t linear = (uint32_t)addr;
    uint32_t npdt = linear >> 22;
    uint32_t npte = (linear >> 12) & 0x3FF;
    pde_t *pdt = (pde_t *)get_cr3();
    pte_t *pet = (pte_t *)(pdt[npdt] & 0xFFFFF000);
    pet[npte] = pet[npte] & ~PAGE_PRESENT;
    invlpg(addr);
}

void
map_vm_page(uint32_t linaddr, uint32_t pyaddr)
{
    uint32_t npdt = linaddr >> 22;
    uint32_t npte = (linaddr >> 12) & 0x3FF;
    pde_t *pdt = (pde_t *)get_cr3();
    if ((pdt[npdt] & PAGE_PRESENT) == 0) {
        int peaddr = (int)alloc_vm_page();
        pdt[npdt] = PAGE_FLOOR(peaddr) | PAGE_PRESENT | PAGE_USER | PAGE_WRITE;
        invlpg((void *)peaddr);
    }
    pte_t *pte = (pte_t *)(pdt[npdt] & 0xFFFFF000);
    pte[npte] = PAGE_FLOOR(pyaddr) | PAGE_PRESENT | PAGE_USER | PAGE_WRITE;
    invlpg((void *)linaddr);
}

void
unmap_vm_page(uint32_t linaddr)
{
    uint32_t npdt = linaddr >> 22;
    uint32_t npte = (linaddr >> 12) & 0x3FF;
    pde_t *pdt = (pde_t *)get_cr3();
    if (pdt[npdt] & PAGE_PRESENT) {
        pte_t *pte = (pte_t *)(pdt[npdt] & 0xFFFFF000);
        pte[npte] = 0;
        invlpg((void *)linaddr);
    }
}

void
switch_vm_page(pde_t *cur_pdt, pde_t *new_pdt)
{
    // 复制内核所在的0 - 4M页表
    pte_t *cur_pte = (pte_t *)PAGE_FLOOR(cur_pdt[0]);
    pte_t *new_pte = NULL;

    if (new_pdt[0] & PAGE_PRESENT)
        new_pte = (pte_t *)PAGE_FLOOR(new_pdt[0]);
    else {
        new_pte = (pte_t *)alloc_vm_page();
    }
    new_pdt[0] = PAGE_FLOOR((uint32_t)new_pte) | PAGE_WRITE | PAGE_USER | PAGE_PRESENT;
    for (int npte = 0; npte < (PAGE_SIZE / sizeof(pte_t)); ++npte) {
        new_pte[npte] = cur_pte[npte];
    }
}

uint32_t
alloc_spage() {
    // 640K的最后一个页面0x9F000不能用?
    static uint32_t tail = 0x9F000;
    tail -= PAGE_SIZE;
    return tail;
}
