#ifndef __MEMORY_H__
#define __MEMORY_H__
#include "defs.h"

#define PAGE_SIZE     (4096)
#define PG_CEILING(addr)    (((addr) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define PG_FLOOR(addr)      ((addr) & ~(PAGE_SIZE - 1))

#define LOG_SIZE(val) ({    \
    int32_t n = (val) - 1;  \
    int32_t ret = 0;        \
    while (n > 0) {         \
        n = n >> 1;         \
        ret += 1;           \
    }                       \
    ret;                    \
})

void
init_memory(uint32_t start, uint32_t end);

//============
// 页面管理
void *
alloc_page(void);

int
free_page(void *page);

int
get_free_space(void);

//=============
// 切片管理
void *
malloc(uint32_t log_size);
int
mfree(void *obj, uint32_t log_size);

//==============
// 对象管理
enum EObjectType {
    ENoneType = 0,
    EBlockBuffer,
    EIndexNode,
    EObjectMax
};

void *
alloc_object(enum EObjectType eObj, uint32_t objsize);

int
free_object(void *obj, enum EObjectType eObj, uint32_t objsize);

#endif
