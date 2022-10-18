#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/second_preim_48_fillme.h"

/*
 * test of the speck48_96
 */
int test_sp48(void) {
	const uint32_t k[4] = {
		0x020100,
		0x0a0908,
		0x121110,
		0x1a1918,
	};
	const uint32_t p[2] = {
		0x6d2073,
		0x696874,
	};
	uint32_t c[2] = {0};
	speck48_96(k, p, c);
	/*printf("%x %x\n", c[1], c[0]);*/
	assert(c[0] == 0x735e10 && c[1] == 0xb6445d);
	return EXIT_SUCCESS;
}


int test_sp48_inv(void){
	return EXIT_SUCCESS;
}


int main () {
	test_sp48();
	printf("ok\n");
	return 0;
}
