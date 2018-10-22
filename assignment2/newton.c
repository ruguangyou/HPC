#include <stdio.h>   // fprintf(), sprintf(), fwrite()
#include <stdlib.h>  // strtol(), exit(), malloc(), calloc(), free()
#include <string.h>  // memcpy()
#include <pthread.h>
#include <math.h>    // cos(), sin()
#include <time.h>    // nanosleep(), timespec
#include "complex.h"

size_t num_threads = 1;
size_t num_lines = 2;
size_t num_items = 4; // num_lines^2
int d;                // simplification: assume degree is less than 10

double *roots_re;     // store the precomputed roots
double *roots_im;

double *re_inits;     // store the initial values of all the data points that are to be computed
double *im_inits;

char **attractors;
char **convergences;
char *row_dones;

char **color_sets;
char **gray_sets;

size_t max_iter = 100;
size_t len_color = 6;  // assume degree less than 10, then the format of color should be "%d %d %d ", e.g. "3 4 5 "
size_t len_gray = 12;  // the format "%d %d %d ", e.g. "058 058 058 "

pthread_mutex_t mutex_item;

void *newton(void *arg);
void *write_to_disc(void *arg);
void (*complex_d)(double *cpx);

int main (int argc, char **argv) {
    // parse arguments
    char *endptr;
    if (argc < 2) {
        fprintf(stderr, "no degree given\n");
        exit(1);
    }
    else if (argc == 2) {
        d = strtol(argv[1], &endptr, 10); // if failed, will return 0
        if (d == 0) {
            fprintf(stderr, "no degree given\n");
            exit(1);
        }
    }
    else if (argc == 3) {
        int i_d = 0;
        int i_tl = 0;
        for (int i = 1; i < 3; ++i) {
            if (argv[i][0] == '-') i_tl = i;
            else i_d = i;
        }
        d = strtol(argv[i_d], &endptr, 10);
        if (i_d == 0 || d == 0) {
            fprintf(stderr, "no degree given\n");
            exit(1);
        }
        if (i_tl == 0) {
            fprintf(stderr, "more than one degrees are given\n");
            exit(1);
        }
        switch (argv[i_tl][1]) {
            case 't' :
                num_threads = strtol(argv[i_tl]+2, &endptr, 10);
                break;
            case 'l':
                num_lines = strtol(argv[i_tl]+2, &endptr, 10);
                break;
        }
    }
    else if (argc == 4) {
        int i_d = 0;
        int i_t, i_l;
        for (int i = 1; i < 4; ++i) {
            if (argv[i][0] == '-') {
                if (argv[i][1] == 't') i_t = i;
                else if (argv[i][1] == 'l') i_l = i;
            }
            else i_d = i;
        }
        d = strtol(argv[i_d], &endptr, 10);
        if (i_d == 0 || d == 0) {
            fprintf(stderr, "no degree given\n");
            exit(1);
        }
        if (d > 13) {  // the degree should not be too large
            fprintf(stderr, "unexpected degree\n");
            exit(1);
        }
        if (i_t == 0 || i_l == 0) {
            fprintf(stderr, "missing some parameter\n");
            exit(1);
        }
        num_threads = strtol(argv[i_t]+2, &endptr, 10);
        num_lines = strtol(argv[i_l]+2, &endptr, 10);
    }
    else {
        fprintf(stderr, "too much parameters\n");
        exit(1);
    }

    if (num_threads < 1) {
        fprintf(stderr, "number of threads must be positive\n");
        exit(1);
    }
    if (num_lines < 2) {
        fprintf(stderr, "number of lines must be greater than 1\n");
        exit(1);
    }
    num_items = num_lines * num_lines;

    pthread_t newton_threads[num_threads];
    pthread_t writing_thread;
    size_t tx;
    int ret;

    // memory allocations, initializations
    roots_re = (double *) malloc(d * sizeof(double));
    roots_im = (double *) malloc(d * sizeof(double));
    // Precompute the exact values of the roots which can be used for comparison during the iteration later.
    // f(x) = x^d - 1, there should be d roots in total
    // the n_th root (n = 0,1,...,d-1) is x_n = re + im*i, where re = cos(2*pi*n/d), im = sin(2*pi*n/d)
    for (int n = 0; n < d; ++n) {
        roots_re[n] = cos(2 * M_PI * n / d);
        roots_im[n] = sin(2 * M_PI * n / d);
    }
    
    re_inits = (double *) malloc(num_lines * sizeof(double));
    im_inits = (double *) malloc(num_lines * sizeof(double));
    // The computation is performed for complex number with real and imaginary part between -2 and +2,
    // the initial values are stored in shared memory that all threads can access to
    double interval = 4.0 / (num_lines - 1);
    double temp;
    for (size_t ix = 0; ix < num_lines; ++ix) {
        temp = -2 + interval * ix;
        re_inits[ix] = temp;
        im_inits[num_lines - 1 - ix] = temp;
    }
    
    size_t i;
    row_dones = (char *) calloc(num_lines, sizeof(char));
    attractors = (char **) malloc(num_items * sizeof(char *));
    convergences = (char **) malloc(num_items * sizeof(char *));

    // preset colors
    // we need d+1 kinds of color, each color has RGB value and corresponding spaces
    char *colors = (char *) malloc((d+1) * len_color * sizeof(char));
    // matrix form
    color_sets = (char **) malloc((d+1) * sizeof(char *));
    for (i = 0; i <= d; ++i)
        color_sets[i] = colors + i*len_color;
    // initialize colors
    for (i = 0; i <= d; ++i)
        for (int j = 0; j < len_color; j += 2) {
            color_sets[i][j] = (i+j) % (d+1) + '0';
            color_sets[i][j+1] = ' ';
        }

    // preset grays
    // according to valgrind memcheck, when using sprintf the string should add one more byte for '\0' as the end
    char *grays = (char *) malloc(max_iter * (len_gray+1) * sizeof(char));
    gray_sets = (char **) malloc(max_iter * sizeof(char *));
    for (size_t i = 0; i < max_iter; ++i)
        gray_sets[i] = grays + i*len_gray;
    // initialize grays
    for (size_t i = 0; i < max_iter; ++i)
        sprintf(gray_sets[i], "%03zu %03zu %03zu ", i+1, i+1, i+1);

    // the pre-hardcoded complex power function
    switch(d) {
        case 1:
            complex_d = complex_d1;
            break;
        case 2:
            complex_d = complex_d2;
            break;
        case 3:
            complex_d = complex_d3;
            break;
        case 4:
            complex_d = complex_d4;
            break;
        case 5:
            complex_d = complex_d5;
            break;
        case 6:
            complex_d = complex_d6;
            break;
        case 7:
            complex_d = complex_d7;
            break;
        case 8:
            complex_d = complex_d8;
            break;
        case 9:
            complex_d = complex_d9;
            break;
        default:
            fprintf(stderr, "unexpected degree\n");
            exit(1);
    }

    // create threads
    for (tx = 0; tx < num_threads; ++tx) {
        size_t *arg = (size_t *) malloc(sizeof(size_t));
        *arg = tx;
        if (ret = pthread_create(&newton_threads[tx], NULL, newton, (void *)arg)) {
            // functions in pthread will return 0 if success
            fprintf(stderr, "Error creating newton thread: %d\n", ret);
            exit(1);
        }
    }
    if (ret = pthread_create(&writing_thread, NULL, write_to_disc, NULL)) {
        fprintf(stderr, "Error creating writing thread: %d\n", ret);
        exit(1);
    }

    for (tx = 0; tx < num_threads; ++tx) {
        if (ret = pthread_join(newton_threads[tx], NULL)) {
            fprintf(stderr, "Error joining newton thread: %d\n", ret);
            exit(1);
        }
    }
    if (ret = pthread_join(writing_thread, NULL)) {
        fprintf(stderr, "Error joining writing thread: %d\n", ret);
        exit(1);
    }

    pthread_mutex_destroy(&mutex_item);

    free(roots_re); free(roots_im);
    free(re_inits); free(im_inits);
    free(attractors); free(convergences);
    free(colors); free(color_sets);
    free(grays); free(gray_sets);
    free(row_dones);

    return 0;
}


void *newton (void *arg) {
    // f(x) = x^d - 1, f'(x) = d * x^(d-1)
    // by making simplification that degree is less than 10,
    // it is allowed to hardcore formula for each degree case,
    // which is largely about finding an expression for the iteration step that is as efficient as possible.
    size_t offset = *((size_t *)arg);
    free(arg);
    
    double re_next, im_next, re_prev, im_prev;
    double re_temp, im_temp, temp;
    double *cpx = (double *) malloc(2 * sizeof(double));
    size_t ix, jx, iter;
    char attr, conv;
    for (ix = offset; ix < num_lines; ix += num_threads) {
        char *attractor = (char *) malloc(len_color * num_lines * sizeof(char));
        char *convergence = (char *) malloc(len_gray * num_lines * sizeof(char));
        attractors[ix] = attractor;
        convergences[ix] = convergence;
        for (jx = 0; jx < num_lines; ++jx) {
            im_prev = im_inits[ix];
            re_prev = re_inits[jx];
            attr = d;  // default, represents the additional root
            conv = max_iter - 1;  // default, represents unconvergence 
            // if x is zero, then f'(x) is zero, and can not be divided, thus categorizing to the additional root
            if (re_prev == 0 && im_prev == 0) {
                memcpy(attractor+jx*len_color, color_sets[attr], len_color);
                memcpy(convergence+jx*len_gray, gray_sets[conv], len_gray);
                continue;
            }

            for (iter = 0; iter < max_iter; ++iter) {
                // compute x^(d-1);
                cpx[0] = re_prev;
                cpx[1] = im_prev;
                complex_d(cpx);
                re_next = cpx[0];
                im_next = cpx[1];

                // absolute value
                re_temp = (re_next < 0) ? (-re_next) : re_next;
                im_temp = (im_next < 0) ? (-im_next) : im_next;
                // if the absolute value of real or imaginary part is bigger than 10^10, abort iteration, categorize to the the additional root
                if (re_temp > 10000000000 || im_temp > 10000000000)
                    break;
                
                // if x is closer than 10^-3 to the origin, abort iteration, categorize to the additional root
                temp = re_next*re_next + im_next*im_next;
                // sqrt(re^2 + im^2) < 10^-3, is just re^2 + im^2 < 10^-6
                if (temp < 0.000001)
                    break;
                
                // if x is closer than 10^-3 to one of the roots, abort iteration, categorize to the corresponding root
                for (int n = 0; n < d; ++n) {
                    re_temp = re_next - roots_re[n];
                    im_temp = im_next - roots_im[n];
                    temp = re_temp*re_temp + im_temp*im_temp;
                    if (temp < 0.000001) { 
                        attr = n;
                        conv = iter;
                        break;
                    }
                }

                if (attr != d)
                    break;

                re_prev = re_next;
                im_prev = im_next;
            }
            memcpy(attractor+jx*len_color, color_sets[attr], len_color);
            memcpy(convergence+jx*len_gray, gray_sets[conv], len_gray);
        }
        pthread_mutex_lock(&mutex_item);
        row_dones[ix] = 1;
        pthread_mutex_unlock(&mutex_item);
    }
    free(cpx);
}


void *write_to_disc (void *arg) {
    /* Write to ppm (Portable PixMap) file
       -----format-----
        P3
        W H  # width height
        M    # the maximum value for wach color, 255 is the maximal in RGB
        (r g b)_WbyH
       ---------------- */
    struct timespec sleep_timespec;
    sleep_timespec.tv_sec = 0;
    sleep_timespec.tv_nsec = 100000;  // sleep 100us
    
    char row_done = 0;
    size_t ix, jx;

    int len_attr_row = len_color * num_lines;
    int len_conv_row = len_gray * num_lines;

    char *local_dones = (char *) calloc(num_lines, sizeof(char));
    
    char buffer[30];
    sprintf(buffer, "newton_attractors_x%d.ppm", d);
    FILE *fa = fopen(buffer, "w");
    sprintf(buffer, "newton_convergence_x%d.ppm", d);
    FILE *fc = fopen(buffer, "w");
    
    fprintf(fa, "P3\n%zu %zu\n%d\n", num_lines, num_lines, d);
    fprintf(fc, "P3\n%zu %zu\n%zu\n", num_lines, num_lines, max_iter);

    for (ix = 0; ix < num_lines; ) {
        pthread_mutex_lock(&mutex_item);
        if ( row_dones[ix] != 0 )
            memcpy(local_dones, row_dones, num_lines);
        pthread_mutex_unlock(&mutex_item);
        
        if (local_dones[ix] == 0) {
            // sleep write_to_disc thread to avoid locking the mutex all the time
            nanosleep(&sleep_timespec, NULL);
            continue;
        }
        
        for ( ; ix < num_lines && local_dones[ix] != 0; ++ix) {
            char *attractor = attractors[ix];
            char *convergence = convergences[ix];
            fwrite(attractor, sizeof(char), len_attr_row, fa);
            fwrite(convergence, sizeof(char), len_conv_row, fc);
            fwrite("\n", sizeof(char), 1, fa);
            fwrite("\n", sizeof(char), 1, fc);
            free(attractor); free(convergence);
        }
    }
    
    free(local_dones);
    fclose(fa);
    fclose(fc);
}

