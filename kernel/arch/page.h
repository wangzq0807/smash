#ifndef __PAGE_H__
#define __PAGE_H__

#define PAGE_SHIFT          12
#define PAGE_SIZE           (1 << PAGE_SHIFT)
#define PAGE_ENTRY_SIZE     (sizeof(int))
#define PAGE_ENTRY_NUM      (PAGE_SIZE/PAGE_ENTRY_SIZE)
#define PAGE_CEILING(addr)  (((addr) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define PAGE_FLOOR(addr)    ((addr) & ~(PAGE_SIZE - 1))
#define PAGE_MARK(addr)     ((addr) & (PAGE_SIZE - 1))

#define PAGE_HUGE_SIZE      (PAGE_ENTRY_NUM*PAGE_SIZE)
#define PAGE_HUGE_CEILING(addr)   (((addr) + PAGE_HUGE_SIZE - 1) & ~(PAGE_HUGE_SIZE - 1))
#define PAGE_HUGE_FLOOR(addr)      ((addr) & ~(PAGE_HUGE_SIZE - 1))

#define PAGE_PRESENT        1
#define PAGE_WRITE          2
#define PAGE_USER           (1 << 2)
#define PAGE_ACCESSED       (1 << 5)
#define PAGE_DIRTY          (1 << 6)

#define PAGE_ENTRY(addr)    (PAGE_FLOOR(addr)|PAGE_PRESENT|PAGE_WRITE|PAGE_USER)
// 缺页类型
// MAX_FD << 1
#define PAGE_ALLOC          (1 << 7)

typedef struct _IrqFrame IrqFrame;

void
on_page_fault(IrqFrame *);

#endif  // __PAGE_H__
