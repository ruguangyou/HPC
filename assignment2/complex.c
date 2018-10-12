#include "complex.h"

void complex_d1 (double *cpx) {
    cpx[0] = 1;
    cpx[1] = 0;
}

void complex_d2 (double *cpx) {
    double a, b, ab;
    a = cpx[0];
    b = cpx[1];
    ab = a*a+b*b;
    cpx[0] = (a+a/ab)/2;
    cpx[1] = (b-b/ab)/2;
}

void complex_d3 (double *cpx) {
    // (a+bi)^2 = (a^2-b^2) + 2*a*bi
    double a, b, a2, b2, ab2;
    a = cpx[0];
    b = cpx[1];
    a2 = a*a-b*b;
    b2 = 2*a*b;
    ab2 = a2*a2+b2*b2;
    cpx[0] = (2*a+a2/ab2)/3;
    cpx[1] = (2*b-b2/ab2)/3;
}

void complex_d4 (double *cpx) {
    double a, b, a2, b2, a3, b3, ab3;
    a = cpx[0];
    b = cpx[1];
    a2 = a*a-b*b;
    b2 = 2*a*b;
    a3 = a*a2-b*b2;
    b3 = a*b2+b*a2;
    ab3 = a3*a3+b3*b3;
    cpx[0] = (3*a+a3/ab3)/4;
    cpx[1] = (3*b-b3/ab3)/4;
}

void complex_d5 (double *cpx) {
    double a, b, a2, b2, a4, b4, ab4;
    a = cpx[0];
    b = cpx[1];
    a2 = a*a-b*b;
    b2 = 2*a*b;
    a4 = a2*a2-b2*b2;
    b4 = 2*a2*b2;
    ab4 = a4*a4+b4*b4;
    cpx[0] = (4*a+a4/ab4)/5;
    cpx[1] = (4*b-b4/ab4)/5;
}

void complex_d6 (double *cpx) {
    double a, b, a2, b2, a4, b4, a5, b5, ab5;
    a = cpx[0];
    b = cpx[1];
    a2 = a*a-b*b;
    b2 = 2*a*b;
    a4 = a2*a2-b2*b2;
    b4 = 2*a2*b2;
    a5 = a*a4-b*b4;
    b5 = a*b4+b*a4;
    ab5 = a5*a5+b5*b5;
    cpx[0] = (5*a+a5/ab5)/6;
    cpx[1] = (5*b-b5/ab5)/6;
}

void complex_d7 (double *cpx) {
    double a, b, a2, b2, a4, b4, a6, b6, ab6;
    a = cpx[0];
    b = cpx[1];
    a2 = a*a-b*b;
    b2 = 2*a*b;
    a4 = a2*a2-b2*b2;
    b4 = 2*a2*b2;
    a6 = a2*a4-b2*b4;
    b6 = a2*b4+b2*a4;
    ab6 = a6*a6+b6*b6;
    cpx[0] = (6*a+a6/ab6)/7;
    cpx[1] = (6*b-b6/ab6)/7;
}

void complex_d8 (double *cpx) {
    double a, b, a2, b2, a4, b4, a6, b6, a7, b7, ab7;
    a = cpx[0];
    b = cpx[1];
    a2 = a*a-b*b;
    b2 = 2*a*b;
    a4 = a2*a2-b2*b2;
    b4 = 2*a2*b2;
    a6 = a2*a4-b2*b4;
    b6 = a2*b4+b2*a4;
    a7 = a*a6-b*b6;
    b7 = a*b6+b*a6;
    ab7 = a7*a7+b7*b7;
    cpx[0] = (7*a+a7/ab7)/8;
    cpx[1] = (7*b-b7/ab7)/8;
}

void complex_d9 (double *cpx) {
    double a, b, a2, b2, a4, b4, a8, b8, ab8;
    a = cpx[0];
    b = cpx[1];
    a2 = a*a-b*b;
    b2 = 2*a*b;
    a4 = a2*a2-b2*b2;
    b4 = 2*a2*b2;
    a8 = a4*a4-b4*b4;
    b8 = a4*b4+b4*a4;
    ab8 = a8*a8+b8*b8;
    cpx[0] = (8*a+a8/ab8)/9;
    cpx[1] = (8*b-b8/ab8)/9;
}

