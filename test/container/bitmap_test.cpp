
#include<iostream>
using namespace std;
#include<gtest/gtest.h>
#include "lib/bitmap.h"

#define BITMAP_SIZE 1024
BitMap testbitmap;
uint8_t bitbuf[BITMAP_SIZE];

TEST(BITMAP, HandlerTrueReturn)
{
    testbitmap.b_nsize = BITMAP_SIZE;
    testbitmap.b_bitbuf = &bitbuf;

    for (int i = 0; i < 100; ++i) {
        int n = bitmap_alloc_bit(&testbitmap);
        EXPECT_EQ(n, i);
        n = bitmap_test_bit(&testbitmap, i);
        EXPECT_EQ(n, 1);
        n = bitmap_test_bit(&testbitmap, i+1);
        EXPECT_EQ(n, 0);
    }
    bitmap_clear_bit(&testbitmap, 31);
    bitmap_clear_bit(&testbitmap, 32);
    bitmap_clear_bit(&testbitmap, 33);
    bitmap_clear_bit(&testbitmap, 40);
    int n = bitmap_alloc_bit(&testbitmap);
    EXPECT_EQ(n, 31);
    n = bitmap_alloc_bit(&testbitmap);
    EXPECT_EQ(n, 32);
    n = bitmap_alloc_bit(&testbitmap);
    EXPECT_EQ(n, 33);
    n = bitmap_alloc_bit(&testbitmap);
    EXPECT_EQ(n, 40);

    int beg = 30;
    int num = 32;
    bitmap_clear_bitrange(&testbitmap, beg, num);
    for (int i = beg; i < beg+num; ++i) {
        // std::cout << i << std::endl;
        int n = bitmap_test_bit(&testbitmap, i);
        EXPECT_EQ(n, 0);
    }
    
    beg = 60;
    num = 64;
    bitmap_set_bitrange(&testbitmap, beg, num);
    for (int i = beg; i < beg+num; ++i) {
        int n = bitmap_test_bit(&testbitmap, i);
        EXPECT_EQ(n, 1);
    }

    n = bitmap_test_bit(&testbitmap, 150);
    EXPECT_EQ(n, 0);
    bitmap_set_bit(&testbitmap, 150);
    n = bitmap_test_bit(&testbitmap, 150);
    EXPECT_EQ(n, 1);

    bitmap_set_bitrange(&testbitmap, 195, 7);
    for (int i = 0; i < 7; ++i)
    {
        n = bitmap_test_bit(&testbitmap, 195+i);
        EXPECT_EQ(n, 1);
    }
    bitmap_set_bitrange(&testbitmap, 205, 10);
    for (int i = 0; i < 10; ++i)
    {
        n = bitmap_test_bit(&testbitmap, 205+i);
        EXPECT_EQ(n, 1);
    }
    for (int i = 202; i < 205; ++i)
    {
        n = bitmap_test_bit(&testbitmap, 200+i);
        EXPECT_EQ(n, 0);
    }

}
