#include "list.h"

void
list_push_back(ListHead *phead, ListNode *pentity)
{
    if (!phead->lh_list) {
        phead->lh_list = pentity;
        phead->lh_list->le_prev = pentity;
        phead->lh_list->le_next = pentity;
        phead->lh_size = 1;
    }
    else {
        ListNode *prev = phead->lh_list->le_prev;
        pentity->le_prev = prev;
        pentity->le_next = phead->lh_list;
        prev->le_next = pentity;
        phead->lh_list->le_prev = pentity;
        phead->lh_size++;
    }
}

void
list_push_front(ListHead *phead, ListNode *pentity)
{
    if (!phead->lh_list) {
        phead->lh_list = pentity;
        phead->lh_list->le_prev = pentity;
        phead->lh_list->le_next = pentity;
        phead->lh_size = 1;
    }
    else {
        ListNode *prev = phead->lh_list->le_prev;
        pentity->le_prev = prev;
        pentity->le_next = phead->lh_list;
        prev->le_next = pentity;
        phead->lh_list->le_prev = pentity;
        phead->lh_list = pentity;
        phead->lh_size++;
    }
}

ListNode *
list_pop_front(ListHead *phead)
{
    if (phead->lh_list == NULL)
        return NULL;
    ListNode *ret = phead->lh_list;
    if (ret->le_next == ret) {
        phead->lh_list = NULL;
        phead->lh_size = 0;
    }
    else {
        phead->lh_list = ret->le_next;
        ret->le_prev->le_next = ret->le_next;
        ret->le_next->le_prev = ret->le_prev;
        phead->lh_size--;
    }
    return ret;
}

void
list_remove_entity(ListHead *phead, ListNode *pentity)
{
    if (phead->lh_list == NULL)
        return;
    if (pentity->le_next == NULL || pentity->le_prev == NULL)
        return;
    ListNode* next = pentity->le_next;
    ListNode* prev = pentity->le_prev;
    if (next != pentity && prev != pentity) {
        next->le_prev = prev;
        prev->le_next = next;
        phead->lh_size--;
        if (pentity == phead->lh_list)
            phead->lh_list = next;
    }
    else {
        phead->lh_list = NULL;
        phead->lh_size=0;
    }
    pentity->le_next = NULL;
    pentity->le_prev = NULL;
}
