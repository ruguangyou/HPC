#include <stdio.h>
#include <stdlib.h>

int main() {
    char s[4] = "1e-5";
    char *endptr;
    int n;
    double m;
    n = strtol(s, &endptr, 10);  // cannot convert "1e5" to 1e5
    m = strtod(s, &endptr);
    printf("%d\n%lf", n, m);
}