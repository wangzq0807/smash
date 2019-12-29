#include "list.h"

void
list_push_back(List *pList, ListNode *pentity)
{
    ListNode *tail = pList->lh_list.le_next;
    pentity->le_prev = tail;
    pentity->le_next = NULL;
    if (tail != NULL)
        tail->le_next = pentity;
    else
        pList->lh_list.le_prev = pentity;
    pList->lh_list.le_next = pentity;
    pList->lh_size++;
}

void
list_push_front(List *pList, ListNode *pentity)
{
    ListNode *head = pList->lh_list.le_prev;
    pentity->le_prev = NULL;
    pentity->le_next = head;
    if (head != NULL)
        head->le_prev = pentity;
    else
        pList->lh_list.le_next = NULL;
    pList->lh_list.le_prev = pentity;
    pList->lh_size++;
}

ListNode *
list_pop_front(List *pList)
{
    ListNode *ret = pList->lh_list.le_prev;
    if (ret != NULL) {
        pList->lh_list.le_prev = ret->le_next;
        if (ret->le_next != NULL)
            ret->le_next->le_prev = NULL;
        else
            pList->lh_list.le_next = NULL;
        pList->lh_size--;
    }
    return ret;
}

void
list_remove_entity(List *pList, ListNode *pentity)
{
    ListNode* next = pentity->le_next;
    ListNode* prev = pentity->le_prev;
    if (next != NULL)
        next->le_prev = prev;
    else
        pList->lh_list.le_next = prev;
    if (prev != NULL)
        prev->le_next = next;
    else
        pList->lh_list.le_prev = next;
    pList->lh_size--;
    pentity->le_next = NULL;
    pentity->le_prev = NULL;
}
