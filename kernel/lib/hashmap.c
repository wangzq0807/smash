#include "hashmap.h"
#include "log.h"

HashMap*
hash_init(HashMap *hmap, HashList *hlist, size_t size, hash_eq_f func)
{
    hmap->hm_table = hlist;
    for (int i = 0; i < size; ++i)
        hmap->hm_table[i].hl_first = 0;
    hmap->hm_size = size;
    hmap->hm_used = 0;
    hmap->hm_eqfunc = func;
    return hmap;
}

static HashNode*
_hashlist_get(HashList *hlist, hash_t hkey, void *target, hash_eq_f eqfunc)
{
    HashNode* iter = hlist->hl_first;
    while (iter != NULL) {
        int eq = 0;
        if (iter->hn_key == hkey) {
            if (eqfunc != NULL)
                eq = eqfunc(iter, target);
            else
                eq = 1;
        }
        if (eq)
            return iter;
        iter = iter->hn_next;
    }
    return NULL;
}

HashNode*
hash_get(HashMap *hmap, hash_t hkey, void* target)
{
    uint32_t bucket = hkey % hmap->hm_size;
    HashList *hlist = &hmap->hm_table[bucket];
    return _hashlist_get(hlist, hkey, target, hmap->hm_eqfunc);
}

HashNode*
hash_put(HashMap *hmap, HashNode* entry, void* target)
{
    size_t bucket = entry->hn_key % hmap->hm_size;
    HashList *hlist = &hmap->hm_table[bucket];
    HashNode *orgnode = _hashlist_get(hlist, entry->hn_key, target, hmap->hm_eqfunc);
    if (orgnode != NULL)
        return NULL;

    HashNode* oldfirst = hlist->hl_first;
    entry->hn_prev = NULL;
    entry->hn_next = oldfirst;
    if (oldfirst != NULL)
        oldfirst->hn_prev = entry;
    hlist->hl_first = entry;
    hmap->hm_used++;
    return oldfirst;
}

HashNode*
hash_rm(HashMap *hmap, hash_t hkey, void* target)
{
    uint32_t bucket = hkey % hmap->hm_size;
    HashList *hlist = &hmap->hm_table[bucket];
    HashNode* orgnode = _hashlist_get(hlist, hkey, target, hmap->hm_eqfunc);
    if (orgnode == NULL)
        return NULL;

    if (orgnode->hn_prev == NULL)
        hlist->hl_first = orgnode->hn_next;
    else
        orgnode->hn_prev->hn_next = orgnode->hn_next;
    if (orgnode->hn_next != NULL)
        orgnode->hn_next->hn_prev = orgnode->hn_prev;
    hmap->hm_used--;
    return orgnode;
}
