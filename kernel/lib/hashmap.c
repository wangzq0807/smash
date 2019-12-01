#include "hashmap.h"

typedef struct
{
    HashNode*       hl_first;
} HashList;

typedef struct
{
    HashList**      hm_table;
    uint32_t        hm_bitsize;
    uint32_t        hm_used;
} HashMapHead;

#define HASH(val, bit)  (((val)*0x9e370001UL) >> (32-bit))

HashMap*
hash_init(uint32_t bitsize)
{
    return NULL;
}

uint32_t
hash_get_size(HashMap hmap)
{
    HashMapHead* phm = (HashMapHead*)hmap;
    return phm->hm_used;
}

static HashNode*
_hashlist_get(HashList hlist, HashKey hkey)
{
    HashNode* iter = hlist.hl_first;
    while (iter != NULL) {
        if (iter->hn_key == hkey)
            return iter;
        iter = iter->hn_next;
    }
    return NULL;
}

HashNode*
hash_get(HashMap hmap, HashKey hkey)
{
    HashMapHead* phm = (HashMapHead*)hmap;
    uint32_t bucket = HASH(hkey, phm->hm_bitsize);
    HashList hlist = (*phm->hm_table)[bucket];
    return _hashlist_get(hlist, hkey);
}

HashNode*
hash_put(HashMap hmap, HashNode* entry)
{
    HashMapHead* phm = (HashMapHead*)hmap;
    uint32_t bucket = HASH(entry->hn_key, phm->hm_bitsize);
    HashList hlist = (*phm->hm_table)[bucket];
    HashNode* orgnode = _hashlist_get(hlist, entry->hn_key);
    if (orgnode != NULL)
        return NULL;

    HashNode* oldfirst = hlist.hl_first;
    hlist.hl_first = entry;
    entry->hn_prev = NULL;
    entry->hn_next = oldfirst;
    oldfirst->hn_prev = entry;
    phm->hm_used++;
    return oldfirst;
}

HashNode*
hash_rm(HashMap hmap, HashKey hkey)
{
    HashMapHead* phm = (HashMapHead*)hmap;
    uint32_t bucket = HASH(hkey, phm->hm_bitsize);
    HashList hlist = (*phm->hm_table)[bucket];
    HashNode* orgnode = _hashlist_get(hlist, hkey);
    if (orgnode != NULL)
        return NULL;

    if (orgnode->hn_prev == NULL)
        (*phm->hm_table)[bucket].hl_first = orgnode->hn_next;
    else
        orgnode->hn_prev->hn_next = orgnode->hn_next;
    if (orgnode->hn_next != NULL)
        orgnode->hn_next->hn_prev = orgnode->hn_prev;
    phm->hm_used--;
    return orgnode;
}
