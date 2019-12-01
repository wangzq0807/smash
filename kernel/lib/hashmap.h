#ifndef __HASHMAP_H__
#define __HASHMAP_H__
#include "sys/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void* HashMap;
typedef uint32_t HashKey;
typedef struct _HashNode
{
    HashKey             hn_key;
    struct _HashNode*   hn_next;
    struct _HashNode*   hn_prev;
} HashNode;

typedef int (*hash_cmp_f)(HashNode* val1, HashKey hkey);

HashMap*
hash_init(uint32_t bitsize);

uint32_t
hash_get_size(HashMap hmap);

HashNode*
hash_get(HashMap hmap, HashKey hkey);

HashNode*
hash_put(HashMap hmap, HashNode* entry);

HashNode*
hash_rm(HashMap hmap, HashKey hkey);

#ifdef __cplusplus
}
#endif

#endif // __HASHMAP_H__
