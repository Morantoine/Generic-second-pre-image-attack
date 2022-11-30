/*#include <assert.h>*/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/second_preim_48_fillme.h"

void ok_test() {
	printf("\033[1;32m");
	printf("\n[Ok] ");
	printf("\033[0m");
}

void failed_test() {
	printf("\033[1;31m");
	printf("\n[Failed] ");
	printf("\033[0m");
}

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
	/*assert(c[0] == 0x735e10 && c[1] == 0xb6445d);*/
	if (c[0] == 0x735e10 && c[1] == 0xb6445d) {
		ok_test();
		printf(__func__);
		return EXIT_SUCCESS;
	} else {
		failed_test();
		printf(__func__);
		printf("\n%x %x\n", c[1], c[0]);
		return EXIT_FAILURE;
	}

}


int test_sp48_inv(void){
	const uint32_t k[4] = {
		0x020100,
		0x0a0908,
		0x121110,
		0x1a1918,
	};
	uint32_t initial_p[2] = {
		0x6d2073,
		0x696874,
	};
	uint32_t p[2] = {
		initial_p[0],
		initial_p[1],
	};
	uint32_t c[2] = {0};
	speck48_96(k, p, c);
	// reinitialize p, just in case, to be sure.
	p[0] = 0;
	p[1] = 0;
	speck48_96_inv(k, c, p);
	/*printf("%x %x\n", initial_p[1], initial_p[0]);*/
	/*printf("%x %x\n", p[1], p[0]);*/
	/*assert(p[0] == initial_p[0] && p[1] == initial_p[1]);*/
	if (p[0] == initial_p[0] && p[1] == initial_p[1]) {
		ok_test();
		printf(__func__);
		return EXIT_SUCCESS;
	} else {
		failed_test();
		printf(__func__);
		printf("\n%x %x\n", p[1], p[0]);
		return EXIT_FAILURE;
	}
}

int test_cs48_dm(void) {
	uint64_t h = 0x010203040506ULL;
	uint32_t m[4] = {
		0, 1, 2, 3
	};

	uint64_t compression= cs48_dm(m, h);
	if (compression == 0x5DFD97183F91ULL) {
		ok_test();
		printf(__func__);
		return EXIT_SUCCESS;
	} else {
		failed_test();
		printf(__func__);
		printf("\n%lx\n", compression);
		return EXIT_FAILURE;
	}
}

int test_fixed_point(void) {
	uint32_t m[4] = {
		1, 2, 3, 4
	};
	// Compute the supposed fixed point
	uint64_t fp_to_check = get_cs48_dm_fp(m);
	// Check that it's correct
	uint64_t compression_result = cs48_dm(m, fp_to_check);
	if (fp_to_check == compression_result) {
		ok_test();
		printf(__func__);
		return EXIT_SUCCESS;
	} else {
		failed_test();
		printf(__func__);
		printf("\n%lx\n is not equal to %lx\n", fp_to_check, compression_result);
		return EXIT_FAILURE;
	}
}

int test_em(void) {
	uint32_t m1[4];
	uint32_t m2[4];
	printf("\n");
	find_exp_mess(m1, m2, 1);
	if (cs48_dm(m1, IV) == get_cs48_dm_fp(m2)) {
		ok_test();
		printf(__func__);
		return EXIT_SUCCESS;
	} else {
		failed_test();
		printf(__func__);
		return EXIT_FAILURE;
	}
}

int main () {
	test_sp48();
	test_sp48_inv();
	test_cs48_dm();
	test_fixed_point();
	test_em();

	printf("\n");
	return 0;
}
