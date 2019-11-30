
#include<iostream>
using namespace std;
#include<gtest/gtest.h>
#include "lib/bitmap.h"

#define BITMAP_SIZE 1024
bitmap_t testbitmap;
bitmap pybm_struct;
uint8_t bitbuf[BITMAP_SIZE];

TEST(BITMAP, HandlerTrueReturn)
{
    // memset(bitbuf, 0, BITMAP_SIZE);
    pybm_struct.b_nsize = BITMAP_SIZE;
    pybm_struct.b_bitbuf = &bitbuf;
    testbitmap = &pybm_struct;

    for (int i = 0; i < 100; ++i) {
        int n = bm_alloc_bit(testbitmap);
        EXPECT_EQ(n, i);
        n = bm_test_bit(testbitmap, i);
        EXPECT_EQ(n, 1);
        n = bm_test_bit(testbitmap, i+1);
        EXPECT_EQ(n, 0);
    }
    bm_clear_bit(testbitmap, 50);
    int n = bm_alloc_bit(testbitmap);
    EXPECT_EQ(n, 50);

    n = bm_test_bit(testbitmap, 150);
    EXPECT_EQ(n, 0);
    bm_set_bit(testbitmap, 150);
    n = bm_test_bit(testbitmap, 150);
    EXPECT_EQ(n, 1);

    bm_set_bitrange(testbitmap, 195, 7);
    for (int i = 0; i < 7; ++i)
    {
        n = bm_test_bit(testbitmap, 195+i);
        EXPECT_EQ(n, 1);
    }
    bm_set_bitrange(testbitmap, 205, 10);
    for (int i = 0; i < 10; ++i)
    {
        n = bm_test_bit(testbitmap, 205+i);
        EXPECT_EQ(n, 1);
    }
    for (int i = 202; i < 205; ++i)
    {
        n = bm_test_bit(testbitmap, 200+i);
        EXPECT_EQ(n, 0);
    }
}
