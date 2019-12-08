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

static int inline is_page_exist(pte_t pte) {
    return pte & PAGE_PRESENT;
}

void init_memory()
{
}

void* alloc_page()
{
    // 640K的最后一个页面0x9F000不能用?
    static uint32_t tail = 0x9F000;
    tail -= PAGE_SIZE;
    return (void *)tail;
}
