#ifndef __SECOND_PREIM__
#define __SECOND_PREIM__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define IV 0x010203040506ULL

void speck48_96(const uint32_t k[4], const uint32_t p[2], uint32_t c[2]);
void speck48_96_inv(const uint32_t k[4], const uint32_t c[2], uint32_t p[2]);
uint64_t cs48_dm(const uint32_t m[4], const uint64_t h);
uint64_t get_cs48_dm_fp(uint32_t m[4]);
void find_exp_mess(uint32_t m1[4], uint32_t m2[4], int verbose);
void attack(int verbose);

#endif // !__SECOND_PREIM__
