#include "lock.h"
#include "asm.h"

int
init_mutex(struct Mutex *mutex)
{
    atomic_swap(&mutex->mt_lock, 0);
    return 0;
}

int
acquire_mutex(struct Mutex *mutex)
{
    if (atomic_swap(&mutex->mt_lock, 1) == 0) {
        // 获取到锁
        return 0;
    }
    return -1;
}

int
release_mutex(struct Mutex *mutex)
{
    atomic_swap(&mutex->mt_lock, 0);
    return 0;
}