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
    const int listlen = 100;
    hashmap.hm_table = new HashList[listlen];
    for (int i = 0; i < listlen; ++i)
        hashmap.hm_table[i].hl_first = 0;
    hashmap.hm_size = listlen;
    hashmap.hm_used = 0;
    hashmap.hm_eqfunc = hash_eq;

    const int tsklen = 205;
    Task ts[tsklen] = {0};

    for (int i = 0; i < tsklen; ++i) {
        ts[i].t_pid = i;
        ts[i].t_hash.hn_key = ts[i].t_pid;
        hash_put(&hashmap, &ts[i].t_hash, &ts[i].t_pid);
    }
    for (int i = 0; i < tsklen; ++i) {
        int pid = i;
        hash_t hh = pid;
        HashNode *node = hash_get(&hashmap, hh, &pid);
        EXPECT_EQ(node->hn_key, i);
    }
    int pid = 5;
    HashNode *node = hash_rm(&hashmap, pid, &pid);
    EXPECT_EQ(node->hn_key, pid);
    node = hash_rm(&hashmap, pid, &pid);
    EXPECT_EQ(node, nullptr);
    hash_for_each(&hashmap, iter) {
        cout << iter->hn_key << endl;
    }
}
