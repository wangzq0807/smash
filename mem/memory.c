#include "memory.h"
#include "log.h"

typedef struct _PageNode PageNode;
struct _PageNode {
    PageNode    *pn_next;
};

typedef struct _PageListHead PageListHead;
struct _PageListHead {
    uint32_t    pl_dummyLock;   // fixme: 链表加锁
    PageNode    *pl_free;
};

PageListHead free_memory;

void
init_memory(uint32_t start, uint32_t end)
{
    if ( (start > end)
        || (size_t)(start) & (PAGE_SIZE - 1)
        || (size_t)(end) & (PAGE_SIZE - 1) ) {
        print("wrong memory range\n");
        return;
    }

    uint32_t next = start;
    free_memory.pl_free = NULL;
    /* 建立空闲页表链表 */
    while ((next + PAGE_SIZE) <= end) {
        free_page((void*)next);
        next += PAGE_SIZE;
    }
}

void *
alloc_page()
{
    /* 返回链表中第一个空闲内存页，同时把头指针指向下一个空闲内存页 */
    PageNode *ret = free_memory.pl_free;
    if (ret != NULL)
        free_memory.pl_free = ret->pn_next;
    return ret;
}

int
free_page(void *page)
{
    if (page == NULL)   return -1;
    if ((size_t)(page) & (PAGE_SIZE - 1)) {
        print("wrong page address\n");
        return -1;
    }
    /* 将新释放的内存页链接到链表头部 */
    PageNode *node = (PageNode *)page;
    node->pn_next = free_memory.pl_free;
    free_memory.pl_free = node;
    return 0;
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


/*************************
 * 内存切片管理：
 * 按2的次幂为单位来分配内存
 *************************/
#define MIN_LOG_SIZE    5       // 对象最小为32字节
#define MAX_LOG_SIZE    10      // 对象最大为1024字节
#define OBJ_LIST_LEN    (MAX_LOG_SIZE - MIN_LOG_SIZE + 1)

static int
mfree_n(void *obj, uint32_t log_size, uint32_t num);

typedef struct _MemSlice MemSlice;
struct _MemSlice {
    MemSlice    *ms_next;
};

typedef struct _MemSliceHead MemSliceHead;
struct _MemSliceHead {
    uint32_t          ms_dummyLock;
    MemSlice  *ms_free;
};
// 为每个2的次幂内存块建一个链表，记录空闲的内存块
MemSliceHead   slice_list[OBJ_LIST_LEN];

void *
malloc(uint32_t log_size)
{
    if (log_size < MIN_LOG_SIZE || log_size > MAX_LOG_SIZE)
        return NULL;
    MemSliceHead *free_list = &slice_list[log_size - MIN_LOG_SIZE];
    if (free_list->ms_free == NULL) {
        void *page = alloc_page();
        if (page == NULL)
            return NULL;
        mfree_n(page, log_size, PAGE_SIZE >> log_size);
    }
    MemSlice *ret = free_list->ms_free;
    free_list->ms_free = ret->ms_next;
    return ret;
}

static int
mfree_n(void *obj, uint32_t log_size, uint32_t num)
{
    if (log_size < MIN_LOG_SIZE || log_size > MAX_LOG_SIZE)
        return -1;
    const uint32_t slice_size = 1 << log_size;
    uint8_t *byte_ptr = (uint8_t*)obj;
    MemSliceHead *free_list = &slice_list[log_size - MIN_LOG_SIZE];
    for (int n = 0; n < num; ++n) {
        ((MemSlice *)byte_ptr)->ms_next = free_list->ms_free;
        free_list->ms_free = ((MemSlice *)byte_ptr);
        byte_ptr += slice_size;
    }
    return 0;
}

int
mfree(void *obj, uint32_t log_size)
{
    return mfree_n(obj, log_size, 1);
}

/*************************
 * 内存对象管理
 * 按对象来分配内存
 *************************/
static int
free_object_n(void *obj, enum EObjectType eObj, uint32_t objsize, uint32_t num);

typedef struct _MemObject MemObject;
struct _MemObject {
    MemObject    *mo_next;
};

typedef struct _MemObjectHead MemObjectHead;
struct _MemObjectHead {
    uint32_t            mo_dummyLock;
    MemObject    *mo_free;
};
MemObjectHead object_list[EObjectMax];

void *
alloc_object(enum EObjectType eObj, uint32_t objsize)
{
    MemObjectHead *free_list = &object_list[eObj];
    if (free_list->mo_free == NULL) {
        void *page = alloc_page();
        if (page == NULL)
            return NULL;
        free_object_n(page, eObj, objsize, PAGE_SIZE / objsize);
    }
    MemObject *ret = free_list->mo_free;
    free_list->mo_free = ret->mo_next;
    return ret; 
}

static int
free_object_n(void *obj, enum EObjectType eObj, uint32_t objsize, uint32_t num)
{
    uint8_t *byte_ptr = (uint8_t*)obj;
    MemObjectHead *free_list = &object_list[eObj];
    for (int n = 0; n < num; ++n) {
        ((MemObject *)byte_ptr)->mo_next = free_list->mo_free;
        free_list->mo_free = ((MemObject *)byte_ptr);
        byte_ptr += objsize;
    }
    return 0;
}

int
free_object(void *obj, enum EObjectType eObj, uint32_t objsize)
{
    return free_object_n(obj, eObj, objsize, 1);
}