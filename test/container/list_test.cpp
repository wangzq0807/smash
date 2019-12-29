#include<iostream>
using namespace std;
#include<gtest/gtest.h>
#include <stddef.h>
#include "lib/list.h"

struct ListNodeInt
{
    int         value;
    ListNode    node;
};

#define LISTNODEINT(list, index)            \
({\
    ListNode* node = list_get(&list, index);\
    LIST_ENTRY(node, ListNodeInt, node);    \
})

#define LISTPOP(list)\
({\
    ListNode *node = list_pop_front(&aaa);    \
    LIST_ENTRY(node, ListNodeInt, node);\
})

TEST(LIST, HandlerTrueReturn)
{
    ListNodeInt nodes[] = {
        {0 },
        {1 },
        {2 },
        {3 },
    };
    List aaa;
    list_init(&aaa);

    list_push_back(&aaa, &nodes[0].node);
    list_push_front(&aaa, &nodes[1].node);
    list_push_back(&aaa, &nodes[2].node);
    list_push_front(&aaa, &nodes[3].node);
    EXPECT_EQ(list_size(&aaa), 4);
    
    ListNodeInt* node = LISTNODEINT(aaa, 0);
    EXPECT_EQ(node->value, 3);
    node = LISTNODEINT(aaa, 1);
    EXPECT_EQ(node->value, 1);
    node = LISTNODEINT(aaa, 2);
    EXPECT_EQ(node->value, 0);
    node = LISTNODEINT(aaa, 3);
    EXPECT_EQ(node->value, 2);

    ListNode* node1 = list_get(&aaa, 4);
    EXPECT_EQ(node1, nullptr);

    node = LISTPOP(aaa);
    EXPECT_EQ(node->value, 3);
    node = LISTPOP(aaa);
    EXPECT_EQ(node->value, 1);
    node = LISTPOP(aaa);
    EXPECT_EQ(node->value, 0);
    node = LISTPOP(aaa);
    EXPECT_EQ(node->value, 2);

    node1 = list_pop_front(&aaa);
    EXPECT_EQ(node1, nullptr);

    list_push_back(&aaa, &nodes[0].node);
    list_push_front(&aaa, &nodes[1].node);
    list_push_back(&aaa, &nodes[2].node);
    list_push_front(&aaa, &nodes[3].node);

    list_remove_entity(&aaa, &nodes[3].node);
    node = LISTNODEINT(aaa, 0);
    EXPECT_EQ(node->value, 1);

    list_remove_entity(&aaa, &nodes[2].node);
    node = LISTNODEINT(aaa, 1);
    EXPECT_EQ(node->value, 0);

    EXPECT_EQ(list_size(&aaa), 2);
}
