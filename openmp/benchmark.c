#include <stdio.h>
#include <stdlib.h>  // system(), strtol()
#include <string.h>
#include "time_diff.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage example: ./benchmark ./cell_distance -t10\n");
        return 1;
    }

    struct timespec start, stop, result;
    double diff = 0;
    
    int iter = 10;
    char cmd[50];

    char *endptr;
    int n_threads = strtol(argv[2]+2, &endptr, 10);

    strcpy(cmd, argv[1]);
    strcat(cmd, " ");
    strcat(cmd, argv[2]);
    strcat(cmd, " > out");
    for (int i = 0; i < iter; ++i) {
        timespec_get(&start, TIME_UTC);
        system(cmd);
        timespec_get(&stop, TIME_UTC);
        time_diff(&start, &stop, &result);
        diff += (double)result.tv_nsec/1000000000 + result.tv_sec;
    }
    printf("[cell_distance -t%d] Average runtime (%d iters): %lf s\n", n_threads, iter, diff/iter);

    return 0;
}