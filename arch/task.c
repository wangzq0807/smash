#include "task.h"
#include "log.h"
#include "asm.h"
#include "lock.h"
#include "memory.h"
#include "page.h"
#include "irq.h"

/* 任务一 */
Task task1;

static void task_1();
static void task_2();

// 竞争资源
Mutex one_mutex;
uint32_t conf_res1 = 1;
uint32_t conf_res2 = 1;

static Task *
get_next_task(Task *cur)
{
    Task *ret = NULL;
    // 从父节点开始，
    // 如果有子进程，则执行子进程
    // 如果没有子进程，则执行兄弟进程
    // 如果没有子进程，且兄弟进程指向的是父进程(说明兄弟进程已穷尽)，则执行父进程的兄弟进程
    if (cur->ts_child_new) {
        ret = cur->ts_child_new;
    }
    else if (cur->ts_older) {
        if (cur->ts_older == cur->ts_parent && cur->ts_parent->ts_older) {
            ret = cur->ts_parent->ts_older;
        }
        else {
            ret = cur->ts_older;
        }
    }
    if (ret == NULL || ret == cur) {
        return NULL;
    }
    else if (ret->ts_state == TS_ZOMBIE) {
        return get_next_task(ret);
    }
    else {
        return ret;
    }
}

void
switch_task()
{
    // NOTE：task1经过跳转到task2，task2再跳转回task1时，会从ljmp的下一条指令开始执行。
    // NOTE：下面这种切换方式是抢占式的，单核情况下，当两个线程需要同步时，加锁必须是原子操作
    // NOTE：对于非抢占式的内核，加锁无需原子操作
    Task *cur = current_task();
    if (cur == NULL)    return; // NOTE:系统未初始化完成

    Task *next = get_next_task(cur);
    if (next != NULL && next != cur)
        switch_tss(&next->ts_tss);
}

#define fork()              \
({                          \
    pid_t pid = 0;          \
    __asm__ volatile (      \
        "movl $2, %%eax \n" \
        "int $0x80 \n"      \
        :"=a"(pid)          \
    );                      \
    pid;                    \
})

#define child_print()       \
({                          \
    int ret = 0;            \
    __asm__ volatile (      \
        "pushl $1 \n"       \
        "movl $1, %%eax \n" \
        "int $0x80 \n"      \
        "popl %%eax \n"     \
        :"=a"(ret)          \
        ::"memory", "esp"   \
    );                      \
    ret;                    \
})

#define exec(path)          \
({                          \
    int ret = 0;            \
    __asm__ volatile (      \
        "pushl $3 \n"       \
        "pushl $2 \n"       \
        "pushl %1 \n"       \
        "movl $11, %%eax \n" \
        "int $0x80 \n"       \
        "addl $12, %%esp \n" \
        :"=a"(ret)          \
        :"r"(path)          \
        :"memory", "esp"    \
    );                      \
    ret;                    \
})

#define delay()         \
{                       \
    int cnt = 100000;   \
    while(cnt--)        \
        pause();        \
}

static void
task_1()
{
    __asm__ volatile (
        "mov $0x17, %%ax \n"
        "mov %%ax, %%ds \n"
        "mov %%ax, %%es \n"
        : : : "%eax"
    );
    pid_t pid = fork();

    if (pid == 0) {
        task_2();
    }
    else {
        while (1) {
            // printk("P");
            delay();
        }
    }
    task_2();
    // while( 1 ) {
    //     if (acquire_mutex(&one_mutex) == 0) {
    //         // 下面是受保护的代码
    //         printk("P");
    //         release_mutex(&one_mutex);
    //     }
    //     else {
    //         // __asm__ volatile("int $0x80");
    //     }
    //     // 延时
    //     // NOTE : 如果从释放锁到重新申请锁的时间过短，
    //     // 那么其他线程获得锁的几率就会非常小。
    //     int cnt = 100000;
    //     while(cnt--)
    //         pause();
    // }
}

static void
task_2()
{
    exec("/one_task");
}

// 第一个进程的堆栈，页表，代码等都位于0-1M内
static void
setup_first_task()
{
    task1.ts_pid = 0;
    // 内核态堆栈
    uint8_t *ks_page = (uint8_t *)alloc_spage();
    ((uint32_t *)ks_page)[0] = (uint32_t)&task1;
    task1.ts_tss.t_SS_0 = KNL_DS;
    task1.ts_tss.t_ESP_0 = (uint32_t)&ks_page[PAGE_SIZE];

    // Note: 任务切换时,CR3不会被自动保存
    pde_t *pdt = (pde_t *)get_cr3();
    task1.ts_tss.t_CR3 = (uint32_t)pdt;
    task1.ts_tss.t_LDT = KNL_LDT;

    // 用户态堆栈
    int us_addr = alloc_pypage();
    map_vm_page(0xFFFF0000, us_addr);
    uint8_t *us_page = (uint8_t *)0xFFFF0000;
    task1.ts_tss.t_ESP = (uint32_t)&us_page[PAGE_SIZE];

    task1.ts_child_new = NULL;
    task1.ts_child_old = NULL;
    task1.ts_older = NULL;
    task1.ts_newer = NULL;
    task1.ts_parent = NULL;
}

void
start_task()
{
    init_mutex(&one_mutex);

    /* 开始第一个进程 */
    setup_first_task();
    start_first_task(&task1.ts_tss, task_1);
}
