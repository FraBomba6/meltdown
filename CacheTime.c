#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <emmintrin.h>
#include <x86intrin.h>

int times[10][10];
int candidates[10];

void getCacheTimes(int cycle) {
    uint8_t array[10 * 4096];
    int junk = 0;
    register uint64_t time1, time2;
    volatile uint8_t *addr;
    int i;

    // Initialize the array
    for (i = 0; i < 10; i++) array[i * 4096] = 1;

    // FLUSH the array from the CPU cache
    for (i = 0; i < 10; i++) _mm_clflush(&array[i * 4096]);

    // Access some of the array items
    array[3 * 4096] = 100;
    array[7 * 4096] = 200;

    for (i = 0; i < 10; i++) {
        addr = &array[i * 4096];
        time1 = __rdtscp(&junk);
        junk = *addr;
        time2 = __rdtscp(&junk) - time1;
        printf("Access time for array[%d*4096]: %d CPU cycles\n", i, (int) time2);
        times[cycle][i] = (int) time2;
    }
}

int main(int argc, const char **argv) {
    int repetition;

    if (argc == 2)
        repetition = (((atoi(argv[1])) < (90)) ? (atoi(argv[1])) : (90));
    else
        repetition = 10;

    for (int i = 0; i < repetition; i++) {
        printf("##### Repetition n° %d #####\n", i + 1);
        getCacheTimes(i);
        printf("\n");
    }

    printf("##### Summary after %d repetitions #####\n", repetition);
    for (int i = 0; i < 10; i++) {
        int sum = 0;
        for (int j = 0; j < repetition; j++) {
            sum += times[j][i];
        }
        printf("Mean access time for array[%d*4096]: %d CPU cycles\n", i, (int) sum / repetition);
        candidates[i] = (int) sum / repetition;
    }
    printf("\n!!!!! In FlushReload script you should pass the argument %d !!!!!\n", (int)candidates[4]/2);

    return 0;
}