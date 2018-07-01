#include "task.h"
#include "log.h"
#include "asm.h"
#include "lock.h"
#include "memory.h"
#include "page.h"
#include "irq.h"

/* 任务一 */
struct Task task1;

static void task_1();
static void task_2();

// 竞争资源
struct Mutex one_mutex;
uint32_t conf_res1 = 1;
uint32_t conf_res2 = 1;

static void
_init_8259A()
{
    /* init_ICW1: 00010001b,多片级联,使用ICW4 */
    outb(0x11, 0x20);
    outb(0x11, 0xA0);
    /* init_ICW2: 中断号 0x20 - 0x27, 0x28 - 0x2f */
    outb(0x20, 0x21);
    outb(0x28, 0xA1);
    /* init_ICW3: 100b, IR2 接从片 */
    outb(0x4, 0x21);        /* 100b, IR2 接从片 */
    outb(0x2, 0xA1);        /* 接主片IR2 */
    /* init_ICW4: 普通全嵌套, 非缓冲, 非自动结束 */
    outb(0x1, 0x21);        /* 普通全嵌套, 非缓冲, 非自动结束 */
    outb(0x1, 0xA1);
}

static void 
_init_timer()
{
    /* 写入控制字, 使用方式3, 计数器0 */
    outb(0x36, 0x43);
    /* 写入计数值，设置每10ms中断一次 */
    uint8_t low = BYTE1(11930);
    outb(low, 0x40);    /* 先写低字节 */
    uint8_t high = BYTE2(11930);
    outb(high, 0x40);   /* 后写高字节 */
}

void
switch_task()
{
    // NOTE：task1经过跳转到task2，task2再跳转回task1时，会从ljmp的下一条指令开始执行。
    // NOTE：下面这种切换方式是抢占式的，单核情况下，当两个线程需要同步时，加锁必须是原子操作
    // NOTE：对于非抢占式的内核，加锁无需原子操作
    struct Task *cur = current_task();
    if (cur->ts_pid == 0 && cur->ts_child_head ) {
        switch_tss(&cur->ts_child_head->ts_tss);
    }
    else if (cur->ts_pid == 1) {
        switch_tss(&cur->ts_parent->ts_tss);
    }
}

#define fork()              \
({                          \
    pid_t pid = 0;          \
    __asm__ volatile (      \
        "movl $0, %%eax \n" \
        "int $0x80 \n"      \
        :"=a"(pid)          \
    );                      \
    pid;                    \
})

#define child_print()       \
({                          \
    int ret = 0;            \
    __asm__ volatile (      \
        "movl $1, %%eax \n" \
        "int $0x80 \n"      \
        :"=a"(ret)          \
    );                      \
    ret;                    \
})

#define exec()              \
({                          \
    int ret = 0;            \
    __asm__ volatile (      \
        "movl $2, %%eax \n" \
        "int $0x80 \n"      \
        :"=a"(ret)          \
    );                      \
    ret;                    \
})

static void
task_1()
{
    return;
    __asm__ volatile (
        "mov $0x17, %%ax \n"
        "mov %%ax, %%ds \n"
        : : : "%eax"
    );
    pid_t pid = fork();

    if (pid == 0)
        task_2();

    while( 1 ) {
        if (acquire_mutex(&one_mutex) == 0) {
            // 下面是受保护的代码
            print("P");
            release_mutex(&one_mutex);
        }
        else {
            // __asm__ volatile("int $0x80");
        }
        // 延时
        // NOTE : 如果从释放锁到重新申请锁的时间过短，
        // 那么其他线程获得锁的几率就会非常小。
        int cnt = 100000;
        while(cnt--)
            pause();
    }
}

static void
task_2()
{
    // exec();
    while( 1 ) {
        if (acquire_mutex(&one_mutex) == 0) {
            child_print();
            release_mutex(&one_mutex);
        }
        else {
            // __asm__ volatile("int $0x80");
        }
        // 延时
        // NOTE : 如果从释放锁到重新申请锁的时间过短，
        // 那么其他线程获得锁的几率就会非常小。
        int cnt = 100000;
        while(cnt--)
            pause();
    }
}

static void
setup_first_task()
{
    task1.ts_pid = 0;
    uint8_t *ks_page = alloc_page();
    uint8_t *us_page = alloc_page();
    ((uint32_t *)ks_page)[0] = (uint32_t)&task1;

    task1.ts_tss.t_SS_0 = KNL_DS;
    task1.ts_tss.t_ESP_0 = (uint32_t)&ks_page[PAGE_SIZE];
    task1.ts_tss.t_ESP = (uint32_t)&us_page[PAGE_SIZE];

    uint32_t *pdt = (uint32_t*)alloc_page();
    uint32_t addr = 0;
    for (int npde = 0; npde < 2; ++npde ){
        uint32_t *pte = (uint32_t*)alloc_page();
        pdt[npde] = PAGE_FLOOR((uint32_t)pte) | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
        for (int i = 0; i < 1024; ++i) {
            pte[i] = PAGE_FLOOR(addr) | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
            addr += PAGE_SIZE;
        }
    }
    // Note: 任务切换时,CR3不会被自动保存
    task1.ts_tss.t_CR3 = (uint32_t)pdt;
    task1.ts_tss.t_LDT = KNL_LDT;

    load_cr3(pdt);
}

void
start_task()
{
    init_mutex(&one_mutex);

    cli();
    setup_idt();
    setup_gdt();

    _init_8259A();
    _init_timer();

    setup_first_task();
    enable_paging();
    sti();
    /* 开始第一个进程 */
    task_1();
    // start_first_task(&task1.ts_tss, task_1);
}
