#include "complex.h"

void complex_power_0 (double *cpx) {
    // (a+bi)^0 = 1 + 0i
    cpx[0] = 1;
    cpx[1] = 0;
}

void complex_power_1 (double *cpx) {
    // (a+bi)^1 = a+bi
}

void complex_power_2 (double *cpx) {
    // (a+bi)^2 = (a^2-b^2) + 2*a*bi
    double a, b;
    a = cpx[0];
    b = cpx[1];
    cpx[0] = a*a - b*b;
    cpx[1] = 2*a*b;
}

void complex_power_3 (double *cpx) {
    // (a+bi)^3 = (a^2-b^2+2*a*bi)*(a+bi) = a*(a^2-3*b^2) + b*(3*a^2-b^2)i
    double a, b, a_sqr, b_sqr;
    a = cpx[0];
    b = cpx[1];
    a_sqr = a*a;
    b_sqr = b*b;
    cpx[0] = a * (a_sqr - 3*b_sqr);
    cpx[1] = b * (3*a_sqr - b_sqr);
}

void complex_power_4 (double *cpx) {
    // (a+bi)^4 = ((a+bi)^2)^2 = (a2+b2i)^2
    double a, b, a2, b2;
    a = cpx[0];
    b = cpx[1];
    a2 = a*a - b*b;
    b2 = 2*a*b;
    cpx[0] = a2*a2 - b2*b2;
    cpx[1] = 2*a2*b2;
}

void complex_power_5 (double *cpx) {
    // (a+bi)^5 = (a+bi)^4*(a+bi) = ((a2+b2i)^2)*(a+bi) = (a4+b4i)*(a+bi) = (a*a4-b*b4) + (a*b4+b*a4)i
    double a, b, a2, b2, a4, b4;
    a = cpx[0];
    b = cpx[1];
    a2 = a*a - b*b;
    b2 = 2*a*b;
    a4 = a2*a2 - b2*b2;
    b4 = 2*a2*b2;
    cpx[0] = a*a4 - b*b4;
    cpx[1] = a*b4 + b*a4;
}

void complex_power_6 (double *cpx) {
    // (a+bi)^6 = ((a+bi)^3)^2 = (a3+b3i)^2
    double a, b, a_sqr, b_sqr, a3, b3;
    a = cpx[0];
    b = cpx[1];
    a_sqr = a*a;
    b_sqr = b*b;
    a3 = a * (a_sqr - 3*b_sqr);
    b3 = b * (3*a_sqr - b_sqr);
    cpx[0] = a3*a3 - b3*b3;
    cpx[1] = 2*a3*b3;
/*
    double a, b, a2, b2, a4, b4;
    a = cpx[0];
    b = cpx[1];
    a2 = a*a - b*b;
    b2 = 2*a*b;
    a4 = a2*a2 - b2*b2;
    b4 = 2*a2*b2;
    cpx[0] = a2*a4 - b2*b4;
    cpx[1] = a2*b4 + b2*a4;
*/
}

void complex_power_7 (double *cpx) {
    // (a+bi)^7 = (a+bi)^6*(a+bi) = (a6+b6i)*(a+bi)
    double a, b, a_sqr, b_sqr, a3, b3, a6, b6;
    a = cpx[0];
    b = cpx[1];
    a_sqr = a*a;
    b_sqr = b*b;
    a3 = a * (a_sqr - 3*b_sqr);
    b3 = b * (3*a_sqr - b_sqr);
    a6 = a3*a3 - b3*b3;
    b6 = 2*a3*b3;
    cpx[0] = a*a6 - b*b6;
    cpx[1] = a*b6 + b*a6;
}

void complex_power_8 (double *cpx) {
    // (a+bi)^8 = (a4+b4i)^2 = ((a2+b2i)^2)^2
    double a, b, a2, b2, a4, b4;
    a = cpx[0];
    b = cpx[1];
    a2 = a*a - b*b;
    b2 = 2*a*b;
    a4 = a2*a2 - b2*b2;
    b4 = 2*a2*b2;
    cpx[0] = a4*a4 - b4*b4;
    cpx[1] = 2*a4*b4;
}

void complex_power_9 (double *cpx) {
    // (a+bi)^9 = (a+bi)^8*(a+bi)
    double a, b, a2, b2, a4, b4, a8, b8;
    a = cpx[0];
    b = cpx[1];
    a2 = a*a - b*b;
    b2 = 2*a*b;
    a4 = a2*a2 - b2*b2;
    b4 = 2*a2*b2;
    a8 = a4*a4 - b4*b4;
    b8 = 2*a4*b4;
    cpx[0] = a*a8 - b*b8;
    cpx[1] = a*b8 + b*a8;
}
