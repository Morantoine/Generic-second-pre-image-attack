#include "../include/second_preim_48_fillme.h"
#include <stdint.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int verbose = 0;
	for(int i=0; i < argc; ++i){
		if (strcmp(argv[i], "-v") == 0) {
			verbose = 1;
		}
    }
	attack(verbose);
	return 0;
}
