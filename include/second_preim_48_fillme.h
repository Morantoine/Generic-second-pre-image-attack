#ifndef __SECOND_PREIM__
#define __SECOND_PREIM__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

void speck48_96(const uint32_t k[4], const uint32_t p[2], uint32_t c[2]);
uint64_t cs48_dm(const uint32_t m[4], const uint64_t h);

#endif // !__SECOND_PREIM__
