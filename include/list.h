#ifndef __LIST_H__
#define __LIST_H__
#include "sys/types.h"

typedef struct _ListEntity ListEntity;
struct _ListEntity {
    ListEntity   *le_prev;
    ListEntity   *le_next; 
};

typedef struct _ListHead ListHead;
struct _ListHead {
    uint32_t        lh_size;
    ListEntity      *lh_head;
    ListEntity      *lh_tail;
};

#define TO_INSTANCE(entity, stype, lname) ({     \
    stype *tmp = (stype *)0;      \
    size_t diff = (size_t)&(tmp->lname) - (size_t)(tmp);\
    (stype *)((size_t)entity - diff);     \
})

static inline void
list_init(ListHead* list)
{
    list->lh_size = 0;
    list->lh_head = NULL;
    list->lh_tail = NULL;
}

static inline ListEntity*
list_get_head(ListHead* phead)
{
    return phead->lh_head;
}

static inline void
list_push_back(ListHead *phead, ListEntity *pentity)
{
    if (!phead->lh_head) {
        phead->lh_head = pentity;
        phead->lh_head->le_prev = pentity;
        phead->lh_head->le_next = pentity;
        phead->lh_size = 1;
    }
    else {
        ListEntity *prev = phead->lh_head->le_prev;
        pentity->le_prev = prev;
        pentity->le_next = phead->lh_head;
        prev->le_next = pentity;
        phead->lh_head->le_prev = pentity;
        phead->lh_size++;
    }
}

static inline void
list_push_front(ListHead *phead, ListEntity *pentity)
{
    if (!phead->lh_head) {
        phead->lh_head = pentity;
        phead->lh_head->le_prev = pentity;
        phead->lh_head->le_next = pentity;
        phead->lh_size = 1;
    }
    else {
        ListEntity *prev = phead->lh_head->le_prev;
        pentity->le_prev = prev;
        pentity->le_next = phead->lh_head;
        prev->le_next = pentity;
        phead->lh_head->le_prev = pentity;
        phead->lh_head = pentity;
        phead->lh_size++;
    }
}

static inline ListEntity *
list_pop_front(ListHead *phead)
{
    if (phead->lh_head == NULL)
        return NULL;
    ListEntity *ret = phead->lh_head;
    if (ret->le_next == ret) {
        phead->lh_head = NULL;
        phead->lh_size = 0;
    }
    else {
        phead->lh_head = ret->le_next;
        ret->le_prev->le_next = ret->le_next;
        ret->le_next->le_prev = ret->le_prev;
        phead->lh_size--;
    }
    return ret;
}

static inline void
list_remove_entity(ListHead *phead, ListEntity *pentity)
{
    if (phead->lh_head == NULL)
        return;
    if (pentity->le_next == NULL || pentity->le_prev == NULL)
        return;
    ListEntity* next = pentity->le_next;
    ListEntity* prev = pentity->le_prev;
    if (next != pentity && prev != pentity) {
        next->le_prev = prev;
        prev->le_next = next;
        phead->lh_size--;
        if (pentity == phead->lh_head)
            phead->lh_head = next;
    }
    else {
        phead->lh_head = NULL;
        phead->lh_size=0;
    }
    pentity->le_next = NULL;
    pentity->le_prev = NULL;
}

#endif  // __LIST_H__
