#include<iostream>
using namespace std;
#include<gtest/gtest.h>
#include "lib/buddy.h"

TEST(BUDDY, HandlerTrueReturn)
{
    Range r[] = {
        { 5, 100},
        { 0, 0}
    };
    buddy_setup(4096, r);
    // EXPECT_EQ(buddy_test(0, 0), 0xfffffffe);
    for (int i = 0; r[i].r_size != 0; ++i) {
        for (int j = 0; j < r[i].r_size; ++j)
            buddy_free(r[i].r_start+j, 1);
    }
    EXPECT_EQ(buddy_test(1, 0), 0xffffffff);

    EXPECT_EQ(buddy_test(8, 0), 0xffff0000);
    for (int i = 0; i < 4096; ++i) {
        int n = buddy_alloc(1);
        EXPECT_EQ(n, i);
    }
    EXPECT_EQ(buddy_test(0, 0), 0xffffffff);
    buddy_free(1, 1);
    EXPECT_EQ(buddy_test(0, 0), 0xfffffffd);
    // buddy_free(1, 1);
    buddy_free(2, 2);
    EXPECT_EQ(buddy_test(1, 0), 0xfffffffd);
    buddy_free(0, 1);
    EXPECT_EQ(buddy_test(1, 0), 0xffffffff);
    EXPECT_EQ(buddy_test(2, 0), 0xfffffffe);

    for (int i = 0; i < 4096; ++i) {
        buddy_free(i, 1);
    }
    EXPECT_EQ(buddy_test(8, 0), 0xffff0000);

    buddy_alloc(32);
    EXPECT_EQ(buddy_test(5, 0), 0xfffffffd);
    buddy_free(16, 15);
    EXPECT_EQ(buddy_test(0, 0), 0xbfffffff);
}

