#ifndef __RANDOM__
#define __RANDOM__

#include "sys/types.h"

void mt_seed(uint32_t seed);

uint32_t mt_rand();

#endif // __RANDOM__
