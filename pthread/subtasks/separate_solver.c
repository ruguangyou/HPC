#include <stdio.h>
#include <stdlib.h>  // strtol()
#include <math.h>
#include "complex.h"

int main() {
    void (*complex_power)(double *cpx);
    double re_init, im_init;
    double re_next, im_next, re_prev, im_prev;
    double re_temp, im_temp, temp, re_temp1, im_temp1;
    double *cpx = malloc(2*sizeof(double));
    int done;
    
    int d = 6;
    double roots[2*d];
    for (int i = 0; i < d; ++i) {
        roots[2*i  ] = cos(2 * M_PI * i / d);
        roots[2*i+1] = sin(2 * M_PI * i / d);
    }

    switch(d-1) {
        case 1:
            complex_power = complex_power_d1;
            break;
        case 2:
            complex_power = complex_power_d2;
            break;
        case 3:
            complex_power = complex_power_d3;
            break;
        case 4:
            complex_power = complex_power_d4;
            break;
        case 5:
            complex_power = complex_power_d5;
            break;
        case 6:
            complex_power = complex_power_d6;
            break;
        case 7:
            complex_power = complex_power_d7;
            break;
        case 8:
            complex_power = complex_power_d8;
            break;
        case 9:
            complex_power = complex_power_d9;
            break;
        default:
            fprintf(stderr, "unexpected degree\n");
            exit(1);
    }

        // f(x) = x^d - 1, f'(x) = d * x^(d-1)
        re_init = -1.97333333333;
        im_init = 0.68666666666;
        printf("%lf %lfi\n", re_init, im_init);


        re_prev = re_init;
        im_prev = im_init;
        done = 0;
        for (int iter = 0; iter < 20; ++iter) {
            // compute x^(d-1);
            cpx[0] = re_prev;
            cpx[1] = im_prev;
            complex_power(cpx);
            re_next = cpx[0];
            im_next = cpx[1];

            // x_next = x - f(x)/f'(x) = (1-1/d) * x + 1/d * 1/x^(d-1)
            // 1/(a+bi) = (a-bi)/(a^2+b^2) = a*c-(b*/c)i, c = 1/(a^2+b^2)
            temp = 1.0 / (d*(re_next*re_next + im_next*im_next));
            re_next = (1.0 - 1.0/d) * re_prev + re_next * temp;
            im_next = (1.0 - 1.0/d) * im_prev - im_next * temp;

            // absolute value
            re_temp = (re_next < 0) ? (-re_next) : re_next;
            im_temp = (im_next < 0) ? (-im_next) : im_next;

            // if the absolute value of real or imaginary part is bigger than 10^10, abort iteration, categorize to the the additional root
            if (re_temp > 10000000000 || im_temp > 10000000000) {
                printf("diverge\n");
                break;
            }

            // if x is closer than 10^-3 to the origin, abort iteration, categorize to the additional root
            re_temp = re_next - re_init;  // delta_re
            im_temp = im_next - im_init;  // delta_im
            // sqrt(d_re^2 + d_im^2) < 10^-3, is just d_re^2 + d_im^2 < 10^-6
            if (re_temp * re_temp + im_temp * im_temp < 0.000001) {
                printf("too close to the origin\n");
                break;
            }
        
            printf("%lf %lfi\n", re_next, im_next);

            // if x is closer than 10^-3 to one of the roots, abort iteration, categorize to the corresponding root
            // the label should be 1, 2, ..., d, and d+1 (additional root). 0 means the item has not done
            for (int n = 0; n < d; ++n) {
                re_temp = re_next - roots[2*n  ];
                im_temp = im_next - roots[2*n+1];
                if (re_temp * re_temp + im_temp * im_temp < 0.000001) {
                    printf("converge\n");
                    done = 1;
                    break;
                }
            }
            if (done)
                break;

            re_prev = re_next;
            im_prev = im_next;
        }

    return 0;
}
