#include <stdio.h>
#include <stdlib.h>  // strtol()
#include <math.h>

int main() {
    
    double re_init, im_init;
    double re_next, im_next, re_prev, im_prev;
    double re_temp, im_temp, temp, re_temp1, im_temp1;
    int done;
    
    int d = 6;
    double roots[2*d];
    for (int i = 0; i < d; ++i) {
        roots[2*i  ] = cos(2 * M_PI * i / d);
        roots[2*i+1] = sin(2 * M_PI * i / d);
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
            if (d > 2) {  // d-1 is even
                re_next = re_prev * re_prev - im_prev * im_prev;
                im_next = 2* re_prev * im_prev;
                re_temp1 = re_next;
                im_temp1 = im_next;
                for (int i = 0; i < (d-1)/2-1; ++i) {
                    re_temp = re_next;
                    im_temp = im_next;
                    re_next = re_temp * re_temp1 - im_temp * im_temp1;
                    im_next = re_temp * im_temp1 + im_temp * re_temp1;
                }
                if (d%2 == 0) {
                    re_temp = re_next;
                    im_temp = im_next;
                    re_next = re_temp * re_prev - im_temp * im_prev;
                    im_next = re_temp * im_prev + im_temp * re_prev;
                }
            }
            else if (d == 2) {
                re_next = re_prev;
                im_next = im_prev;
            }
            else {
                re_next = 1;
                im_next = 0;
            }

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
