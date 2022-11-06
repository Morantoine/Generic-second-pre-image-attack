#include "../include/second_preim_48_fillme.h"
#include <bits/types/clock_t.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

int main(int argc, char *argv[])
{
	int verbose = 0;
	for(int i=0; i < argc; ++i){
		if (strcmp(argv[i], "-v") == 0) {
			verbose = 1;
		}
    }
	double total_time = 0;
	for (int i = i; i < 21; i++) {
		clock_t start = clock();
		printf("Round %i\n", i);
		attack(verbose);
		clock_t stop = clock();
		total_time += (stop - start) / ((double) CLOCKS_PER_SEC);
	}
	printf("On average, %f sec\n", total_time/((double) 20));
	return 0;
}
