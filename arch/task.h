#ifndef __TASK_H__
#define __TASK_H__
#include "sys/types.h"
#include "x86.h"

typedef struct _Task Task;
struct _Task{
    pid_t           ts_pid;
    gid_t           ts_gid;
    uid_t           ts_uid;
    uint32_t        ts_state;
    uint32_t        ts_lock;
    time_t          ts_time;
    error_t         ts_exit;

    Task    *ts_parent;
    Task    *ts_child_head;
    Task    *ts_child_tail;
    Task    *ts_next;
    Task    *ts_prev;
    Task    *ts_hash_prev;
    Task    *ts_hash_next;

    X86TSS  ts_tss;
};

void
start_task();

void
switch_task();

#endif // __TASK_H__