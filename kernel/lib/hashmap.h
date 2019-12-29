#ifndef __HASHMAP_H__
#define __HASHMAP_H__
#include "stdint.h"
#include "stddef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef size_t  hash_t;
//#define HASH(val, bit)  (((val)*0x9e370001UL) >> (32-bit))
typedef struct _HashNode
{
    hash_t              hn_key;
    struct _HashNode*   hn_next;
    struct _HashNode*   hn_prev;
} HashNode;

typedef int (*hash_eq_f)(HashNode* node, void* target);

typedef struct
{
    HashNode*       hl_first;
} HashList;

typedef struct
{
    HashList*       hm_table;
    uint32_t        hm_size;
    uint32_t        hm_used;
    hash_eq_f       hm_eqfunc;
} HashMap;

HashMap*
hash_init(uint32_t bitsize, hash_eq_f func);

static inline uint32_t
hash_get_size(HashMap *hmap) {
    return hmap->hm_used;
}

HashNode*
hash_get(HashMap *hmap, hash_t hkey, void* target);

HashNode*
hash_put(HashMap *hmap, HashNode* entry, void* target);

HashNode*
hash_rm(HashMap *hmap, hash_t hkey, void* target);

#ifdef __cplusplus
}
#endif

#endif // __HASHMAP_H__
