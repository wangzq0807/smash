#include "random.h"

#define MT_M 397
#define MT_N 624
#define MT_A 0x9908B0DF
#define MT_F 1812433253
#define MT_R 31
#define MT_LOWER ((1ul<<MT_R)-1)
#define MT_UPPER (1ul<<MT_R)

uint32_t MT[MT_N];
uint32_t mt_index;

void
mt_seed(uint32_t seed)
{
    MT[0] = seed;
    for (int i = 1; i < MT_N; ++i)
        MT[i] = MT_F * (MT[i-1] ^ (MT[i-1]>>30)) + i;
    mt_index = MT_N;
}

static void
mt_twist()
{
    for (int i = 0; i < MT_N; ++i) {
        uint32_t x = (MT[i] & MT_UPPER) + (MT[(i + 1) % MT_N] & MT_LOWER);
        uint32_t xA = x >> 1;
        if (x & 1)
            xA ^= MT_A;
        MT[i] = MT[(i + MT_M) % MT_N] ^ xA;
    }
    mt_index = 0;
}

uint32_t
mt_rand()
{
    if (mt_index >= MT_N)
        mt_twist();
    
    uint32_t val = MT[mt_index++];
    val ^= (val >> 11);
    val ^= (val << 7) & 0x9D2C5680;
    val ^= (val << 15) & 0xEFC60000;
    val ^= (val >> 18);

    return val;
}
