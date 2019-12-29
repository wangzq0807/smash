#include<iostream>
using namespace std;
#include<gtest/gtest.h>
#include "lib/hashmap.h"

typedef struct
{
    int         t_pid;
    HashNode    t_hash;
} Task;

#define HASH_ENTRY(entity, stype, lname) ({     \
    stype *tmp = (stype *)0;      \
    size_t diff = (size_t)&(tmp->lname) - (size_t)(tmp);\
    (stype *)((size_t)entity - diff);     \
})

int hash_eq(HashNode* node, void* target)
{
    Task* ts = HASH_ENTRY(node, Task, t_hash);
    int pid = *(int*)target;
    if (ts->t_pid == pid)
        return 1;
    else
        return 0;
}

TEST(HASHMAP, HandlerTrueReturn)
{
    HashMap hashmap;
    hashmap.hm_table = new HashList[1024];
    for (int i = 0; i < 1024; ++i)
        hashmap.hm_table[i].hl_first = 0;
    hashmap.hm_size = 1024;
    hashmap.hm_used = 0;
    hashmap.hm_eqfunc = hash_eq;

    Task ts[2048] = {0};

    for (int i = 0; i < 2048; ++i) {
        ts[i].t_pid = i;
        ts[i].t_hash.hn_key = ts[i].t_pid;
        hash_put(&hashmap, &ts[i].t_hash, &ts[i].t_pid);
    }
    for (int i = 0; i < 2048; ++i) {
        int pid = i;
        hash_t hh = pid;
        HashNode *node = hash_get(&hashmap, hh, &pid);
        EXPECT_EQ(node->hn_key, i);
    }
}
