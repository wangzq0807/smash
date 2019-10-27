#ifndef __TASK_H__
#define __TASK_H__
#include "sys/types.h"
#include "x86.h"
#include "fs/vfile.h"

#define MAX_FD      64
#define FD_MASK(val)    ((val)&(MAX_FD-1))

#define TS_RUN      1
#define TS_ZOMBIE   2
#define TS_SLEEP    3

typedef struct _Task Task;
struct _Task {
    pid_t           ts_pid;
    gid_t           ts_gid;
    uid_t           ts_uid;
    uint32_t        ts_state;
    uint32_t        ts_lock;
    time_t          ts_time;
    error_t         ts_exit;
    size_t          ts_size;
    VFile           *ts_filps[MAX_FD];
    dev_t           ts_cdev;
    ino_t           ts_cinode;

    Task    *ts_parent;
    Task    *ts_child_new;      // 最新的子进程
    Task    *ts_child_old;      // 最旧的子进程
    Task    *ts_older;          // 较老的兄弟进程
    Task    *ts_newer;          // 较新的兄弟进程
    ListEntity      ts_hash_link;
    Task    *ts_wait;           // 等待的父进程

    X86TSS  ts_tss;
};

void
start_task();

void
switch_task();

void
sleep(Task *ts);

void
wakeup(Task *ts);

Task *
new_task(Task *parent);

int
delete_task(Task *task);

Task *
get_task(pid_t pid);

#endif // __TASK_H__