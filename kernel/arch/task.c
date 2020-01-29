#include "task.h"
#include "lib/log.h"
#include "asm.h"
#include "lib/lock.h"
#include "memory.h"
#include "page.h"
#include "irq.h"
#include "sys/fcntl.h"
#include "unistd.h"
#include "mem/frame.h"

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

    Task *next = cur;
    do {
        next = get_next_task(next);
    } while (next != NULL && next->ts_state != TS_RUN);

    if (next != NULL && next != cur) {
        // pdt_t cur_pde = (pdt_t)pym2vm(cur->ts_tss.t_CR3);
        // pdt_t next_pde = (pdt_t)pym2vm(next->ts_tss.t_CR3);
        // switch_vm_page(cur_pde, next_pde);
        switch_tss(&next->ts_tss);
    }
}

#define sys_pause()         \
({                          \
    int ret = 0;            \
    __asm__ volatile (      \
        "pushl $0 \n"       \
        "movl $19, %%eax \n" \
        "int $0x80 \n"      \
        "addl $4, %%esp \n" \
        :"=a"(ret)          \
    );                      \
    ret;                    \
})

void
task_1()
{
    __asm__ volatile (
        "mov $0x17, %%ax \n"
        "mov %%ax, %%ds \n"
        "mov %%ax, %%es \n"
        : : : "%eax"
    );
    open("/dev/tty", O_RDONLY, 0);
    open("/dev/tty", O_WRONLY, 0);
    pid_t pid = fork();

    if (pid == 0) {
        task_2();
    }
    else {
        while (1) {
            sys_pause();
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
    execve("/bin/bash", 0, NULL);
}

// 第一个进程的堆栈，页表，代码等都位于0-1M内
static void
setup_first_task()
{
    task1.ts_pid = 0;
    // 内核态堆栈
    task_init_kstack(&task1);
    // 用户态堆栈
    task_init_ustack(&task1);

    // Note: 任务切换时,CR3不会被自动保存
    pdt_t pdt = get_pdt();
    task1.ts_tss.t_CR3 = vm2pym((vm_t)pdt);
    task1.ts_tss.t_LDT = KNL_LDT;

    task1.ts_child_new = NULL;
    task1.ts_child_old = NULL;
    task1.ts_older = NULL;
    task1.ts_newer = NULL;
    task1.ts_parent = NULL;
    task1.ts_state = TS_RUN;
    task1.ts_cdev = ROOT_DEVICE;
    task1.ts_cinode = ROOT_INODE;
}

#define TSK_HASH_LEN    64
#define HASH(pid)   (pid & (TSK_HASH_LEN - 1))
HashMap hashmap;
HashList hashlist[TSK_HASH_LEN];

int pid_eq(HashNode* node, void* pid) {
    Task* ts = LIST_ENTRY(node, Task, ts_hash_link);
    if (ts->ts_pid == (hash_t)pid)
        return 1;
    else
        return 0;
}

void
start_task()
{
    init_mutex(&one_mutex);

    hash_init(&hashmap, hashlist, TSK_HASH_LEN, pid_eq);

    /* 开始第一个进程 */
    setup_first_task();
    start_first_task(&task1.ts_tss, task_1);
}

void
sleep(Task *ts)
{
    if (ts == NULL) return;
    ts->ts_state = TS_SLEEP;
    switch_task();
}

void
wakeup(Task *ts)
{
    if (ts == NULL) return;
    if (ts->ts_state != TS_ZOMBIE)
        ts->ts_state = TS_RUN;
}

static Task *
_get_hash_entity(pid_t pid)
{
    hash_t hashpid = HASH(pid);
    HashNode* node = hash_get(&hashmap, hashpid, (size_t*)pid);
    if (node != NULL) {
        return LIST_ENTRY(node, Task, ts_hash_link);
    }
    return NULL;
}

static int
_remove_hash_entity(Task *task)
{
    hash_t hashpid = HASH(task->ts_pid);
    hash_rm(&hashmap, hashpid, (size_t*)task->ts_pid);
    return 0;
}

static int
_put_hash_entity(Task *task)
{
    Task *org = _get_hash_entity(task->ts_pid);
    if (org != NULL)
        _remove_hash_entity(org);
    task->ts_hash_link.hn_key = HASH(task->ts_pid);
    hash_put(&hashmap, &task->ts_hash_link, (size_t*)task->ts_pid);
    return 0;
}

static void
setup_task_link(Task *parent_task, Task *new_task)
{
    new_task->ts_parent = parent_task;
    new_task->ts_child_new = NULL;
    new_task->ts_child_old = NULL;
    if (parent_task->ts_child_new != NULL)
        new_task->ts_older = parent_task->ts_child_new;
    else
        new_task->ts_older = parent_task;
    new_task->ts_newer = parent_task;

    // NOTE: 下面的操作未完成前，不能发生进程切换
    if (parent_task->ts_child_old == NULL)
        parent_task->ts_child_old = new_task;
    if (parent_task->ts_child_new != NULL) {
        parent_task->ts_child_new->ts_newer = new_task;
    }
    parent_task->ts_child_new = new_task;
    _put_hash_entity(new_task);
}

static void
unlink_task(Task *task)
{
    Task *older = task->ts_older;
    Task *newer = task->ts_newer;
    Task *parent = task->ts_parent;
    // NOTE: 下面的操作未完成前，不允许发生进程切换
    if (older != NULL && newer != NULL) {
        if (newer != parent)
            newer->ts_older = task->ts_child_new;
        else
            parent->ts_child_new = task->ts_child_new;
        if (older != parent)
            older->ts_newer = task->ts_child_old;
        else
            parent->ts_child_old = task->ts_child_old;
    }

    Task *iter = task->ts_child_new;
    while (iter != NULL) {
        iter->ts_parent = task->ts_parent;
        if (iter == task->ts_child_old)
            break;
        iter = iter->ts_older;
    }
    _remove_hash_entity(task);
}

Task *
new_task(Task *parent)
{
    static pid_t nextpid = 1;
    Task *new_task = (Task *)vm_alloc();
    new_task->ts_pid = nextpid++;
    setup_task_link(parent, new_task);
    return new_task;
}

int
delete_task(Task *task)
{
    // NOTE: 被删除的一定不能是当前进程
    if (task == current_task()) return -1;

    unlink_task(task);
    // 回收内核堆栈, NOTE: esp要减1
    vm_free((void *)PAGE_FLOOR(task->ts_tss.t_ESP_0 - 1));
    // TODO: 回收内存
    // 回收task
    vm_free(task);

    return 0;
}

Task *
get_task(pid_t pid)
{
    return _get_hash_entity(pid);
}

// 内核态堆栈
int
task_init_kstack(Task *task) {
    vm_t kstack = (vm_t)vm_alloc();
    ((size_t *)kstack)[0] = (size_t)task;
    
    task->ts_tss.t_SS_0  = KNL_DS;
    task->ts_tss.t_ESP_0 = kstack + PAGE_SIZE;
    return 0;
}

// 用户态堆栈
int
task_init_ustack(Task *task) {
    vm_t ustack = (vm_t)vm_alloc_stack();
    task->ts_tss.t_ESP = ustack + PAGE_SIZE;
    task->ts_tss.t_EBP = ustack + PAGE_SIZE;
    return 0;
}