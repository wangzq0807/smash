#ifndef __TASK_H__
#define __TASK_H__
#include "defs.h"
#include "x86.h"

struct Task {
    pid_t           ts_pid;
    gid_t           ts_gid;
    uid_t           ts_uid;
    uint32_t        ts_state;
    uint32_t        ts_lock;
    time_t          ts_time;
    error_t         ts_exit;

    struct Task     *ts_parent;
    struct Task     *ts_child_head;
    struct Task     *ts_child_tail;
    struct Task     *ts_next;
    struct Task     *ts_prev;
    struct Task     *ts_hash_prev;
    struct Task     *ts_hash_next;

    struct X86TSS   ts_tss;
};

void
start_task();

void
switch_task();

#endif // __TASK_H__