#include <stdio.h>
#include <stdlib.h>  // system(), strtol()
#include <string.h>
#include "time_diff.h"

int main(int argc, char **argv) {
    if (argc != 7) {
        printf("Usage example: ./benchmark ./heat_diffusion 1000 100 -i1e10 -d0.02 -n20\n");
        return 1;
    }

    struct timespec start, stop, result;
    double diff = 0;
    
    int iter = 10;
    char cmd[100];
    strcpy(cmd, argv[1]);
    for (int i = 2; i < 7; i++) {
        strcat(cmd, " ");
        strcat(cmd, argv[i]);
    }
    for (int i = 0; i < iter; ++i) {
        timespec_get(&start, TIME_UTC);
        system(cmd);
        timespec_get(&stop, TIME_UTC);
        time_diff(&start, &stop, &result);
        diff += (double)result.tv_nsec/1000000000 + result.tv_sec;
    }
    printf("[%s] Average runtime (%d iters): %lf s\n", cmd, iter, diff/iter);

    return 0;
}