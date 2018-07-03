#ifndef __LIST_H__
#define __LIST_H__
#include "defs.h"

struct ListEntry {
    struct ListEntry    *le_prev;
    struct ListEntry    *le_next; 
};

struct ListHead {
    int                 lh_lock;
    struct ListEntry    *lh_list;
};

static inline void
push_back(struct ListHead *phead, struct ListEntry *pentry)
{
    if (!phead->lh_list) {
        phead->lh_list = pentry;
        phead->lh_list->le_prev = pentry;
        phead->lh_list->le_next = pentry;
    }
    else {
        struct ListEntry *prev = phead->lh_list->le_prev;
        pentry->le_prev = prev;
        pentry->le_next = phead->lh_list;
        prev->le_next = pentry;
        phead->lh_list->le_prev = pentry;
    }
}

static inline void
push_front(struct ListHead *phead, struct ListEntry *pentry)
{
    push_back(phead, pentry);
    phead->lh_list = pentry;
}

static inline void
remove_entity(struct ListHead *phead, struct ListEntry *pentry)
{
    if (phead->lh_list == NULL)
        return;
    if (pentry->le_next == NULL || pentry->le_prev == NULL)
        return;
    struct ListEntry* next = pentry->le_next;
    struct ListEntry* prev = pentry->le_prev;
    if (next != pentry && prev != pentry) {
        next->le_prev = prev;
        prev->le_next = next;
        if (pentry == phead->lh_list)
            phead->lh_list = next;
    }
    else {
        phead->lh_list = NULL;
    }
    pentry->le_next = NULL;
    pentry->le_prev = NULL;
}

#endif  // __LIST_H__
