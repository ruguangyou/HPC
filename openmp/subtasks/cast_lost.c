#include <stdio.h>
#include "time_diff.h"

int main() {
    struct timespec start, stop, result;
    long sum;

    timespec_get(&start, TIME_UTC);
    for (int i = 0; i < 1; i++) {
        sum = 0;
        for (int j = 0; j < 1e9; j++)
            sum += j;
    }
    timespec_get(&stop, TIME_UTC);
    time_diff(&start, &stop, &result);
    printf("runtime without cast: %lf s\n", (double)result.tv_nsec/1000000000 + result.tv_sec);

    timespec_get(&start, TIME_UTC);
    for (int i = 0; i < 1; i++) {
        sum = 0;
        for (int j = 0; j < 1e9; j++)
            sum += (double)j;
    }
    timespec_get(&stop, TIME_UTC);
    time_diff(&start, &stop, &result);
    printf("runtime with cast: %lf s\n", (double)result.tv_nsec/1000000000 + result.tv_sec);

    return 0;
}