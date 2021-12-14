#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <emmintrin.h>
#include <x86intrin.h>

uint8_t array[256 * 4096];
int temp;
char secret = 94;
/* cache hit time threshold assumed*/
int CACHE_HIT_THRESHOLD;
#define DELTA 1024

int repetition;

void flushSideChannel() {
    int i;

    // Write to array to bring it to RAM to prevent Copy-on-write
    for (i = 0; i < 256; i++) array[i * 4096 + DELTA] = 1;

    //flush the values of the array from cache
    for (i = 0; i < 256; i++) _mm_clflush(&array[i * 4096 + DELTA]);
}

void victim() {
    temp = array[secret * 4096 + DELTA];
}

int reloadSideChannel() {
    int junk = 0;
    register uint64_t time1, time2;
    volatile uint8_t *addr;
    int i;
    for (i = 0; i < 256; i++) {
        addr = &array[i * 4096 + DELTA];
        time1 = __rdtscp(&junk);
        junk = *addr;
        time2 = __rdtscp(&junk) - time1;
        if (time2 <= CACHE_HIT_THRESHOLD) {
            printf("array[%d*4096 + %d] is in cache.\n", i, DELTA);
            printf("The Secret = %d.\n", i);
            return i;
        }
    }
    return -1;
}

int main(int argc, const char **argv) {
    if (argc >= 2 && argc <= 3)
        CACHE_HIT_THRESHOLD = atoi(argv[1]);
    else if (argc == 3)
        repetition = atoi(argv[2]);
    else {
        CACHE_HIT_THRESHOLD = 80;
        repetition = 20;
    }

    int accuracy = 0;

    for (int i = 0; i < repetition; i++) {
        printf("##### Try n° %d #####\n", i + 1);
        flushSideChannel();
        victim();
        int guess = reloadSideChannel();
        if (guess == secret)
            accuracy++;
        else
            printf("Failed to retrieve the correct secret index\n");
    }

    printf("\n***** Accuracy: %.2f *****\n\n", accuracy/(double)repetition);
    return 0;
}
