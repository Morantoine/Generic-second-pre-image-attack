#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

#include "../include/second_preim_48_fillme.h"
#include "../include/xoshiro256starstar.h"

#define ROTL24_16(x) ((((x) << 16) ^ ((x) >> 8)) & 0xFFFFFF)
#define ROTL24_3(x) ((((x) << 3) ^ ((x) >> 21)) & 0xFFFFFF)

#define ROTL24_8(x) ((((x) << 8) ^ ((x) >> 16)) & 0xFFFFFF)
#define ROTL24_21(x) ((((x) << 21) ^ ((x) >> 3)) & 0xFFFFFF)

// N = sqrt(2^48) = 2^24 car on veut une collision sur 48 bits.
// Avec le paradoxe des anniversaires, on peut faire une attaque "meet on the middle search"
// et avoir 1 chance sur 2 de trouver une collision avec sqrt(nombre de possibilitées)
/*#define N 16777216*/
#define N 16777216

unsigned long long boucles = 0;

typedef struct key_message {
	uint64_t hash;
	uint32_t message[4];
	int index;
}KeyMessage;

/*
 * the 96-bit key is stored in four 24-bit chunks in the low bits of k[0]...k[3]
 * the 48-bit plaintext is stored in two 24-bit chunks in the low bits of p[0], p[1]
 * the 48-bit ciphertext is written similarly in c
 */

void speck48_96(const uint32_t k[4], const uint32_t p[2], uint32_t c[2])
{
	uint32_t rk[23];
	uint32_t ell[3] = {k[1], k[2], k[3]};

	rk[0] = k[0];

	c[0] = p[0];
	c[1] = p[1];

	/* full key schedule */
	for (unsigned i = 0; i < 22; i++)
	{
		uint32_t new_ell = ((ROTL24_16(ell[0]) + rk[i]) ^ i) & 0xFFFFFF; // addition (+) is done mod 2**24
		rk[i+1] = ROTL24_3(rk[i]) ^ new_ell;
		ell[0] = ell[1];
		ell[1] = ell[2];
		ell[2] = new_ell;
	}

	for (unsigned i = 0; i < 23; i++)
	{
		c[0] = ((ROTL24_16(c[0]) + c[1]) ^ rk[i]) & 0xFFFFFF;
		c[1] = ROTL24_3(c[1]) ^ c[0];
	}

	return;
}


/* the inverse cipher */
void speck48_96_inv(const uint32_t k[4], const uint32_t c[2], uint32_t p[2])
{
	uint32_t rk[23];
	uint32_t ell[3] = {k[1], k[2], k[3]};
	uint32_t buff;

	rk[0] = k[0];

	p[0] = c[0];
	p[1] = c[1];

	/* full key schedule */
	for (unsigned i = 0; i < 22; i++)
	{
		uint32_t new_ell = ((ROTL24_16(ell[0]) + rk[i]) ^ i) & 0xFFFFFF; // addition (+) is done mod 2**24
		rk[i+1] = ROTL24_3(rk[i]) ^ new_ell;
		ell[0] = ell[1];
		ell[1] = ell[2];
		ell[2] = new_ell;
	}
	
	for (int i = 22; i >= 0; i--)
	{
		buff = p[0];
		p[0] = ROTL24_8(((p[0] ^ rk[i]) - ROTL24_21(p[0] ^ p[1])) & 0xFFFFFF);
		p[1] = ROTL24_21(buff ^ p[1]);
	}

	return;
}

/* The Davies-Meyer compression function based on speck48_96,
 * using an XOR feedforward
 * The input/output chaining value is given on the 48 low bits of a single 64-bit word,
 * whose 24 lower bits are set to the low half of the "plaintext"/"ciphertext" (p[0]/c[0])
 * and whose 24 higher bits are set to the high half (p[1]/c[1])
 */
uint64_t cs48_dm(const uint32_t m[4], const uint64_t h)
{
	uint32_t m1[4] = {m[3], m[2], m[1], m[0]};
	uint32_t hashed_tab[2] = {0};
	uint32_t h_tab[2] = {h & 0xFFFFFF, h >> 24 & 0xFFFFFF};
	speck48_96(m1, h_tab, hashed_tab);		
	uint64_t hashed = ((uint64_t) hashed_tab[1]) << 24 | (uint64_t) hashed_tab[0];
	return hashed ^ h;
}

/* assumes message length is fourlen * four blocks of 24 bits, each stored as the low bits of 32-bit words
 * fourlen is stored on 48 bits (as the 48 low bits of a 64-bit word)
 * when padding is include, simply adds one block (96 bits) of padding with fourlen and zeros on higher pos */
uint64_t hs48(const uint32_t *m, uint64_t fourlen, int padding, int verbose)
{
	uint64_t h = IV;
	const uint32_t *mp = m;

	for (uint64_t i = 0; i < fourlen; i++)
	{
		h = cs48_dm(mp, h);
		if (verbose)
			printf("@%lu : %06X %06X %06X %06X => %06lX\n", i, mp[0], mp[1], mp[2], mp[3], h);
		mp += 4;
	}
	if (padding)
	{
		uint32_t pad[4];
		pad[0] = fourlen & 0xFFFFFF;
		pad[1] = (fourlen >> 24) & 0xFFFFFF;
		pad[2] = 0;
		pad[3] = 0;
		h = cs48_dm(pad, h);
		if (verbose)
			printf("@%lu : %06X %06X %06X %06X => %06lX\n", fourlen, pad[0], pad[1], pad[2], pad[3], h);
	}

	return h;
}

/* Computes the unique fixed-point for cs48_dm for the message m */
uint64_t get_cs48_dm_fp(uint32_t m[4])
{
	uint32_t m1[4] = {m[3], m[2], m[1], m[0]}; 
	// We just need to compute the preimage of 0 by the block cypher
	uint32_t c[2] = {0};
	uint32_t p[2];

	speck48_96_inv(m1, c, p);
	uint64_t formatted_result = ((uint64_t) p[1]) << 24 | (uint64_t) p[0];
	return formatted_result;
}

static int compar(const void* a, const void* b) {
	return ((KeyMessage*)a)->hash > ((KeyMessage*)b)->hash;
}

/*
 * Dichotomy in O(log n)
 */
int64_t find(uint64_t elem, KeyMessage tab[], uint64_t len) {
	// On peut améliorer l'algorithme avec une "interpolation search" qui est en O(log(log(N)))
	// Puisque les éléments sont répartis avec une loi uniforme. On pourrait statistiquement 
	// chercher la position de l'élément et gagner du temps.
    int left_index = 0, right_index = len - 1;
    int index;
    while (left_index < right_index) {
		boucles++;
        index = left_index - ((left_index - right_index) >> 1); 
        if (tab[index].hash == elem)
            return index;
        if (tab[index].hash < elem) {
            left_index = index + 1;
		} else {
            right_index = index - 1;
		}
    }
    return -1;
}

uint64_t positive(uint64_t a) {
	return a * (a > 0);
}

uint64_t major(uint64_t a, uint64_t majorant) {
	if (a > majorant) {
		return majorant;
	} else {
		return a;
	}
}


/* Finds a two-block expandable message for hs48, using a fixed-point
 * That is, computes m1, m2 s.t. hs48_nopad(m1||m2) = hs48_nopad(m1||m2^*),
 * where hs48_nopad is hs48 with no padding */
void find_exp_mess(uint32_t m1[4], uint32_t m2[4], int verbose) {
	printf("Looking for a fixed point !\n");
	clock_t start_of_all = clock();
	KeyMessage* tab_m1;
	// On alloue car créer un tableau de 16 millions d'éléments faisait un segfault.
	// On choisit un tableau mais on aurait pu faire une table de hashage.
	// Les performances sont correctes, donc on est resté sur cette solution.
	if (verbose) {printf("Allocating memory for the meet-in-the-middle search...\n");}
	if (!(tab_m1 = malloc(N * sizeof(KeyMessage)))) {
		fprintf(stderr, "Error: calloc\n");
		return;
	}
	uint64_t m1_64_tmp[2];
	uint32_t m1_32_tmp[4];
	// compute N possible chaining values for N random first-block messages m1
	uint32_t i;
	if (verbose) {
		printf("Calculating 2^24 hashes...\n");
	}
	for (i = 0; i < N; i++) {
		m1_64_tmp[0] = xoshiro256starstar_random();
		m1_64_tmp[1] = xoshiro256starstar_random();
		m1_32_tmp[0] = m1_64_tmp[0] & 0xffffffff;
		m1_32_tmp[1] = m1_64_tmp[0] & 0xffffffff;
		m1_32_tmp[2] = m1_64_tmp[1] & 0xffffffff00000000;
		m1_32_tmp[3] = m1_64_tmp[1] & 0xffffffff00000000;

		// saved in a hash-map
		tab_m1[i].hash =  cs48_dm(m1_32_tmp, IV);
		tab_m1[i].message[0] = m1_32_tmp[0];
		tab_m1[i].message[1] = m1_32_tmp[1];
		tab_m1[i].message[2] = m1_32_tmp[2];
		tab_m1[i].message[3] = m1_32_tmp[3];
		/*printf("%lu\n", tab_m1[i].hash);*/

		/*printf("%lu\n", tab_m1[i].hash);*/
		/*printf("%u\n\n", tab_m1[i].message[0]);*/
	}
	if (verbose) {
		printf("Sorting the hashes...\n");
	}
	qsort(tab_m1, N, sizeof(KeyMessage), compar);

	if (verbose) {
		printf("Looking for a collision...\n");
	}
	clock_t start = clock();
	i = 0;
	uint64_t elem64[2];
	uint32_t elem[4];
	do {
		elem64[0] = xoshiro256starstar_random();
		elem64[1] = xoshiro256starstar_random();
		elem[0] = elem64[0] & 0xffffffff;
		elem[1] = elem64[1] & 0xffffffff;
		elem[2] = elem64[0] & 0xffffffff00000000;
		elem[3] = elem64[1] & 0xffffffff00000000;
		i++;
		/*printf("%lu en %u itérations\n", elem, i);*/
	} while (find(get_cs48_dm_fp(elem), tab_m1, N) == -1);
	uint64_t collision = get_cs48_dm_fp(elem);
	if (verbose) {
		printf("Found a fixed point %lu in %u iterations\n", collision, i);
		printf("Took %f seconds in total\n\n", (clock() - start_of_all) / (double)CLOCKS_PER_SEC);
	}

	m2[0] = elem[0];
	m2[1] = elem[1];
	m2[2] = elem[2];
	m2[3] = elem[3];

	m1[0] = tab_m1[find(collision, tab_m1, N)].message[0];
	m1[1] = tab_m1[find(collision, tab_m1, N)].message[1];
	m1[2] = tab_m1[find(collision, tab_m1, N)].message[2];
	m1[3] = tab_m1[find(collision, tab_m1, N)].message[3];

	free(tab_m1);
}



void attack(int verbose)
{
	// Message for which we want to find a second preimage
	const uint64_t NUM_BLOCKS = 1 << 18;
    uint32_t mess[4 * NUM_BLOCKS];
    for (int i = 0; i < (1 << 20); i += 4) {
        mess[i + 0] = i;
        mess[i + 1] = 0;
        mess[i + 2] = 0;
        mess[i + 3] = 0;
    }
    uint64_t hash = hs48(mess, NUM_BLOCKS, 1, 0);
    assert(hash == 0x7CA651E182DBULL);
	// Compute the chaining hashes of the message and store them in a sorted array
	printf("Starting the main attack !\n");
	if (verbose) {printf("Allocating memory for the intermediate hashes\n");} 
	KeyMessage* tab_original_msg;
	if (!(tab_original_msg = malloc(NUM_BLOCKS * sizeof(KeyMessage)))) {
		fprintf(stderr, "Error: calloc");
		return;
	}
	// We compute and store all the hashes of the blocks
	tab_original_msg[0].hash = cs48_dm(mess, IV);
	tab_original_msg[0].message[0] = mess[0];
	tab_original_msg[0].message[1] = mess[1];
	tab_original_msg[0].message[2] = mess[2];
	tab_original_msg[0].message[3] = mess[3];
	tab_original_msg[0].index = 0;
	for (int i = 1; i < NUM_BLOCKS; i++) {
		// saved in a hash-map
		tab_original_msg[i].hash = cs48_dm(mess + 4* i, tab_original_msg[i - 1].hash);
		tab_original_msg[i].message[0] = mess[4*i + 0];
		tab_original_msg[i].message[1] = mess[4*i + 1];
		tab_original_msg[i].message[2] = mess[4*i + 2];
		tab_original_msg[i].message[3] = mess[4*i + 3];
		tab_original_msg[i].index = i;
	}
	// We now sort the hashmap to find a collision
	if (verbose) {printf("Sorting intemediate hashes...\n");} 
	qsort(tab_original_msg, NUM_BLOCKS, sizeof(KeyMessage), compar);
	// Generate one pair on messages
	// corresponding to an expandable messag
	uint32_t m1[4];
	uint32_t m2[4];
	uint32_t cm[4];
	uint64_t fixed_point;
	uint64_t h;
	uint64_t cmbuffer[2];
	find_exp_mess(m1, m2, verbose);
	if (verbose) {printf("Trying to find a block that collides from the fixed point...\n");} 
	fixed_point = cs48_dm(m1, IV);
	// Randomly generate cm until we collide on any intermediate hash
	int i = 0;
	do {
		i++;
		if ((i % 100000000 == 0) & verbose) {
			printf("%iM tries already\n", i / 1000000);
		}
		cmbuffer[0] = xoshiro256starstar_random();
		cmbuffer[1] = xoshiro256starstar_random();
		cm[0] = cmbuffer[0] & 0xffffffff;
		cm[1] = cmbuffer[1] & 0xffffffff;
		cm[2] = cmbuffer[0] & 0xffffffff00000000;
		cm[3] = cmbuffer[1] & 0xffffffff00000000;
		h = cs48_dm(cm, fixed_point);
	} while ((find(h, tab_original_msg, NUM_BLOCKS) == -1)); 
	if (verbose) {printf("Found colliding block in %i iterations\n", i);}
	// Then we need the index of the block we collided on
	int index = tab_original_msg[find(h, tab_original_msg, NUM_BLOCKS)].index;
	if (verbose) {printf("at index %u\n", index);}
	assert(h == tab_original_msg[find(h, tab_original_msg, NUM_BLOCKS)].hash);
	// We can now recreate the message
	if (verbose) {printf("Reassembling the second message\n");}
    uint32_t *mess2= malloc(4 * NUM_BLOCKS * sizeof(uint32_t));

	mess2[0] = m1[0];
	mess2[1] = m1[1];
	mess2[2] = m1[2];
	mess2[3] = m1[3];
	for (int i = 4; i < 4 * index; i += 4) {
		mess2[i + 0] = m2[0];
		mess2[i + 1] = m2[1];
		mess2[i + 2] = m2[2];
		mess2[i + 3] = m2[3];
	}
	for (int j = 0; j < 4; j++) {
		mess2[4 * index+ j] = cm[j];
	}
	for (int i = 4 * (index + 1); i < (1 << 20); i += 4) {
		mess2[i + 0] = i;
		mess2[i + 1] = 0;
		mess2[i + 2] = 0;
		mess2[i + 3] = 0;
	}
	printf("Hash for the second preimage is %lx\n", hs48(mess2, NUM_BLOCKS, 1, 0));
	printf("Original hash was %lx\n", hash);
	printf("Attack succeeded !");

	int saved_stdout;
	saved_stdout = dup(1);
	char *name = "second_message_verbose.txt";
    int fd = open(name, O_WRONLY | O_CREAT, 0644);
    if (fd == -1) {
        perror("open failed");
        exit(1);
    }

    if (dup2(fd, 1) == -1) {
        perror("dup2 failed"); 
        exit(1);
    }

	hs48(mess2, NUM_BLOCKS, 1, 1);

	name = "original_message_verbose.txt";
    fd = open(name, O_WRONLY | O_CREAT, 0644);
    if (fd == -1) {
        perror("open failed");
        exit(1);
    }

    if (dup2(fd, 1) == -1) {
        perror("dup2 failed"); 
        exit(1);
    }

	hs48(mess, NUM_BLOCKS, 1, 1);
	/* Restore stdout */
	dup2(saved_stdout, 1);
	
	close(saved_stdout);
}
