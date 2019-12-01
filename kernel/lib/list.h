#ifndef __LIST_H__
#define __LIST_H__
#include "sys/types.h"

typedef struct _ListNode {
    struct _ListNode   *le_prev;
    struct _ListNode   *le_next; 
} ListNode;

typedef struct {
    uint32_t        lh_size;
    ListNode      *lh_list;
} ListHead;

#define LIST_ENTRY(entity, stype, lname) ({     \
    stype *tmp = (stype *)0;      \
    size_t diff = (size_t)&(tmp->lname) - (size_t)(tmp);\
    (stype *)((size_t)entity - diff);     \
})

static inline void
list_init(ListHead* list)
{
    list->lh_size = 0;
    list->lh_list = NULL;
}

static inline ListNode*
list_get_head(ListHead* phead)
{
    return phead->lh_list;
}

void
list_push_back(ListHead *phead, ListNode *pentity);

void
list_push_front(ListHead *phead, ListNode *pentity);

ListNode *
list_pop_front(ListHead *phead);

void
list_remove_entity(ListHead *phead, ListNode *pentity);

#endif  // __LIST_H__
