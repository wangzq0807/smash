#ifndef __MEMORY_H__
#define __MEMORY_H__
#include "sys/types.h"

#define PAGE_LOG_SIZE   12
#define PAGE_SIZE     ( 1 << PAGE_LOG_SIZE)
#define PAGE_CEILING(addr)    (((addr) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define PAGE_FLOOR(addr)      ((addr) & ~(PAGE_SIZE - 1))

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
uint32_t
alloc_pypage(void);

int
get_pypage_refs(uint32_t page);

int
add_pypage_refs(uint32_t page);

int
release_pypage(uint32_t page);

int
get_free_space(void);


#endif
