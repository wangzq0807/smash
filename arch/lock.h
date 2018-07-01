#ifndef __LOCK__
#define __LOCK__
#include "defs.h"

struct Mutex {
    uint32_t    mt_lock;
};

int init_mutex(struct Mutex *mutex);

int acquire_mutex(struct Mutex *mutex);

int release_mutex(struct Mutex *mutex);

#endif