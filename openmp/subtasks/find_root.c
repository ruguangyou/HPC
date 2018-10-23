#include <stdio.h>
#include <math.h>
#include "time_diff.h"

int main() {
    struct timespec start, stop, result;

    short left, mid, right, root;
    double square = 500000;
    size_t squared_index[3466];
    for (size_t i = 0; i < 3466; i++)
        squared_index[i] = i * i;

    timespec_get(&start, TIME_UTC);
    for (int i = 0; i < 10000; i++) {
        left = 0; right = 3465;
        while(1) {
            mid = (left + right) / 2;
            if (square >= squared_index[mid]) {
                if (square < squared_index[mid+1])
                    break;
                else
                    left = mid;
            }
            else {
                right = mid;
            }
        }
    }
    timespec_get(&stop, TIME_UTC);
    time_diff(&start, &stop, &result);
    printf("binary search time (10000 iters): %lf\n", (double)result.tv_nsec/1000000000 + result.tv_sec);

    timespec_get(&start, TIME_UTC);
    for (int i = 0; i < 10000; i++)
        root = (short) sqrt(square);
    timespec_get(&stop, TIME_UTC);
    time_diff(&start, &stop, &result);
    printf("sqrt time (10000 iters): %lf\n", (double)result.tv_nsec/1000000000 + result.tv_sec);

    return 0;
}