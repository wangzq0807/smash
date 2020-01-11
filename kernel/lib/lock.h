#ifndef __LOCK__
#define __LOCK__
#include "sys/types.h"

typedef struct _Mutex Mutex;
struct _Mutex {
    uint32_t    mt_lock;
};

int init_mutex(Mutex *mutex);

int acquire_mutex(Mutex *mutex);

int release_mutex(Mutex *mutex);

#endif // __LOCK__