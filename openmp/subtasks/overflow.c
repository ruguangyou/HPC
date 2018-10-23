#include <stdio.h>
#include <math.h>

int main() {
    short x, y;
    double d;

    x = 40000;
    y = -20000;
    printf("x: %hi  y: %hi\n", x, y);
    d = sqrt(x*x + y*y);
    printf("d: %lf\n", d);

    return 0;
}