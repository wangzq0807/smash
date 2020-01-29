#ifndef __LIST_H__
#define __LIST_H__
#include "stdint.h"
#include "stddef.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
typedef struct _ListNode {
    struct _ListNode   *le_prev;
    struct _ListNode   *le_next;
} ListNode;

typedef struct {
    size_t        lh_size;
    ListNode      lh_list;
} List;

#define LIST_ENTRY(entity, stype, lname) ({     \
    stype *tmp = (stype *)0;      \
    size_t diff = (size_t)&(tmp->lname) - (size_t)(tmp);\
    (stype *)((size_t)entity - diff);     \
})

static inline void
list_init(List* list) {
    list->lh_size = 0;
    list->lh_list.le_prev = NULL;
    list->lh_list.le_next = NULL;
}

static inline size_t
list_size(List* list) {
    return list->lh_size;
}

static inline ListNode*
list_get(List* pList, int index) {
    if (index >= pList->lh_size)
        return NULL;
    int cur = 0;
    ListNode* itr = pList->lh_list.le_prev;
    while (cur != index && itr != NULL) {
        itr = itr->le_next;
        ++cur;
    }
    return itr;
}

#define list_for_each(pList, itr)   \
    for (ListNode *(itr) = (pList)->lh_list.le_prev; (itr) != NULL; (itr) = (itr)->le_next)

void
list_push_back(List *pList, ListNode *pentity);

void
list_push_front(List *pList, ListNode *pentity);

ListNode *
list_pop_front(List *pList);

void
list_remove_entity(List *pList, ListNode *pentity);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif  // __LIST_H__
