#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct key_message {
    uint64_t hash;
    uint32_t message[4];
}KeyMessage;

int find(int e, int* tab, int N) {
    int l = 0, r = N - 1;
    int i;
    while ( l <= r ) {
        i = (l + r) / 2;
        if (tab[i] == e)
            return 1;
        if (tab[i] < e )
            l = i+1;
        else
            r = i-1;
    }
    return 0;
}

int cmpfct(const void* km1, const void *km2) {
    //printf("%lu", ((const KeyMessage*)km1)->hash);
    const KeyMessage* km1_s = *(const KeyMessage**) km1;
    const KeyMessage* km2_s = *(const KeyMessage**) km2;
    return km1_s->hash - km2_s->hash;
}

int main() {
    KeyMessage m1 = malloc
    printf("%lu", tab[0]->hash);
    qsort(tab, 2, sizeof(KeyMessage), cmpfct);
    //if (find(10, tab, 10)) {
    //    printf("Trouvé\n");
    //} else {
    //    printf("Pas trouvé\n");
    //}
}
