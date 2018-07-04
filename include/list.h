#ifndef __LIST_H__
#define __LIST_H__
#include "defs.h"

struct ListEntity {
    struct ListEntity   *le_prev;
    struct ListEntity   *le_next; 
};

struct ListHead {
    int                 lh_lock;
    struct ListEntity   *lh_list;
};

#define TO_INSTANCE(entity, stype, lname) ({     \
    struct stype *tmp = (struct stype *)0;      \
    size_t diff = (size_t)&(tmp->lname) - (size_t)(tmp);\
    (struct stype *)((size_t)entity - diff);     \
})

static inline void
push_back(struct ListHead *phead, struct ListEntity *pentity)
{
    if (!phead->lh_list) {
        phead->lh_list = pentity;
        phead->lh_list->le_prev = pentity;
        phead->lh_list->le_next = pentity;
    }
    else {
        struct ListEntity *prev = phead->lh_list->le_prev;
        pentity->le_prev = prev;
        pentity->le_next = phead->lh_list;
        prev->le_next = pentity;
        phead->lh_list->le_prev = pentity;
    }
}

static inline void
push_front(struct ListHead *phead, struct ListEntity *pentity)
{
    if (!phead->lh_list) {
        phead->lh_list = pentity;
        phead->lh_list->le_prev = pentity;
        phead->lh_list->le_next = pentity;
    }
    else {
        struct ListEntity *prev = phead->lh_list->le_prev;
        pentity->le_prev = prev;
        pentity->le_next = phead->lh_list;
        prev->le_next = pentity;
        phead->lh_list->le_prev = pentity;
        phead->lh_list = pentity;
    }
}

static inline struct ListEntity *
pop_front(struct ListHead *phead)
{
    if (phead->lh_list == NULL)
        return NULL;
    struct ListEntity *ret = phead->lh_list;
    if (phead->lh_list == ret) {
        phead->lh_list = NULL;
    }
    else {
        ret->le_prev->le_next = ret->le_next;
        ret->le_next->le_prev = ret->le_prev;
    }
    return ret;
}

static inline void
remove_entity(struct ListHead *phead, struct ListEntity *pentity)
{
    if (phead->lh_list == NULL)
        return;
    if (pentity->le_next == NULL || pentity->le_prev == NULL)
        return;
    struct ListEntity* next = pentity->le_next;
    struct ListEntity* prev = pentity->le_prev;
    if (next != pentity && prev != pentity) {
        next->le_prev = prev;
        prev->le_next = next;
        if (pentity == phead->lh_list)
            phead->lh_list = next;
    }
    else {
        phead->lh_list = NULL;
    }
    pentity->le_next = NULL;
    pentity->le_prev = NULL;
}

#endif  // __LIST_H__
