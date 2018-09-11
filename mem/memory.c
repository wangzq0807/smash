#include "memory.h"
#include "log.h"
#include "list.h"

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
}

static void
_new_freelist(uint32_t addr)
{
    // 一个页面用来建立空闲页的链表(大约会新增1M的可用内存)
    PageNode *node_list = (PageNode*)addr;
    int node_num = PAGE_SIZE / sizeof(PageNode);
    node_num = MIN(node_num, (max_end - addr) >> PAGE_LOG_SIZE);
    for (int i = 0; i < node_num; i++ ) {
        node_list[i].pn_page = (addr >> PAGE_LOG_SIZE) + i;
        node_list[i].pn_refs = 0;
        node_list[i].pn_next = &node_list[i+1];
    }
    node_list[node_num - 1].pn_next = NULL;
    node_list[0].pn_refs = 1;
    _put_hash_entity(&node_list[0]);

    free_memory.pl_free = node_list[0].pn_next;
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
    if (node != NULL)   return -1;

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

int
get_free_space()
{
    PageNode *node = free_memory.pl_free;
    int cnt = 0;
    while (node != NULL) {
        cnt++;
        node = node->pn_next;
    }
    return cnt;
}


// /*************************
//  * 内存切片管理：
//  * 按2的次幂为单位来分配内存
//  *************************/
// #define MIN_LOG_SIZE    5       // 对象最小为32字节
// #define MAX_LOG_SIZE    10      // 对象最大为1024字节
// #define OBJ_LIST_LEN    (MAX_LOG_SIZE - MIN_LOG_SIZE + 1)

// static int
// mfree_n(void *obj, uint32_t log_size, uint32_t num);

// typedef struct _MemSlice MemSlice;
// struct _MemSlice {
//     MemSlice    *ms_next;
// };

// typedef struct _MemSliceHead MemSliceHead;
// struct _MemSliceHead {
//     uint32_t          ms_dummyLock;
//     MemSlice  *ms_free;
// };
// // 为每个2的次幂内存块建一个链表，记录空闲的内存块
// MemSliceHead   slice_list[OBJ_LIST_LEN];

// void *
// malloc(uint32_t log_size)
// {
//     if (log_size < MIN_LOG_SIZE || log_size > MAX_LOG_SIZE)
//         return NULL;
//     MemSliceHead *free_list = &slice_list[log_size - MIN_LOG_SIZE];
//     if (free_list->ms_free == NULL) {
//         void *page = alloc_pypage();
//         if (page == NULL)
//             return NULL;
//         mfree_n(page, log_size, PAGE_SIZE >> log_size);
//     }
//     MemSlice *ret = free_list->ms_free;
//     free_list->ms_free = ret->ms_next;
//     return ret;
// }

// static int
// mfree_n(void *obj, uint32_t log_size, uint32_t num)
// {
//     if (log_size < MIN_LOG_SIZE || log_size > MAX_LOG_SIZE)
//         return -1;
//     const uint32_t slice_size = 1 << log_size;
//     uint8_t *byte_ptr = (uint8_t*)obj;
//     MemSliceHead *free_list = &slice_list[log_size - MIN_LOG_SIZE];
//     for (int n = 0; n < num; ++n) {
//         ((MemSlice *)byte_ptr)->ms_next = free_list->ms_free;
//         free_list->ms_free = ((MemSlice *)byte_ptr);
//         byte_ptr += slice_size;
//     }
//     return 0;
// }

// int
// mfree(void *obj, uint32_t log_size)
// {
//     return mfree_n(obj, log_size, 1);
// }

// /*************************
//  * 内存对象管理
//  * 按对象来分配内存
//  *************************/
// static int
// free_object_n(void *obj, enum EObjectType eObj, uint32_t objsize, uint32_t num);

// typedef struct _MemObject MemObject;
// struct _MemObject {
//     MemObject    *mo_next;
// };

// typedef struct _MemObjectHead MemObjectHead;
// struct _MemObjectHead {
//     uint32_t            mo_dummyLock;
//     MemObject    *mo_free;
// };
// MemObjectHead object_list[EObjectMax];

// void *
// alloc_object(enum EObjectType eObj, uint32_t objsize)
// {
//     MemObjectHead *free_list = &object_list[eObj];
//     if (free_list->mo_free == NULL) {
//         void *page = alloc_pypage();
//         if (page == NULL)
//             return NULL;
//         free_object_n(page, eObj, objsize, PAGE_SIZE / objsize);
//     }
//     MemObject *ret = free_list->mo_free;
//     free_list->mo_free = ret->mo_next;
//     return ret; 
// }

// static int
// free_object_n(void *obj, enum EObjectType eObj, uint32_t objsize, uint32_t num)
// {
//     uint8_t *byte_ptr = (uint8_t*)obj;
//     MemObjectHead *free_list = &object_list[eObj];
//     for (int n = 0; n < num; ++n) {
//         ((MemObject *)byte_ptr)->mo_next = free_list->mo_free;
//         free_list->mo_free = ((MemObject *)byte_ptr);
//         byte_ptr += objsize;
//     }
//     return 0;
// }

// int
// free_object(void *obj, enum EObjectType eObj, uint32_t objsize)
// {
//     return free_object_n(obj, eObj, objsize, 1);
// }
