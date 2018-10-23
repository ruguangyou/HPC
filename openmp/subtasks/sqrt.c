#include <stdio.h> 

float sqrt1 (float n) {
    unsigned int i = *(unsigned int*) &n;
    i += 127 << 23;
    i >>= 1;
    return *(float*) &i;
}

int main() {
    float n = 223.89;
    printf("sqrt of %f: %f\n", n, sqrt1(n));
}