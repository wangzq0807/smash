#include<iostream>
using namespace std;
#include<gtest/gtest.h>
#include "lib/hashmap.h"

typedef struct
{
    hash_cmp_f      hm_hashcmpf;
    int             hm_size;
    int             hm_bucket_len;
    int*            hm_bucket;
} HashMapHead;

typedef struct
{
    int         t_pid;
    HashNode    t_hash;
} Task;

int HashCompair(HashNode* val1, HashKey hkey) {

}

TEST(HASHMAP, HandlerTrueReturn)
{
    HashMapHead hashmap;
    hashmap.hm_hashcmpf = NULL;
    hashmap.hm_size = 0;
    hashmap.hm_bucket_len = 1024;
    hashmap.hm_bucket = new int[1024];

    Task ts;
    ts.t_pid = 10;
    hash_put(&hashmap, &ts.t_hash);

    hash_get(&hashmap, (HashKey)ts.t_pid);
}
