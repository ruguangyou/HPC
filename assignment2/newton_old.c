#include <stdio.h>
#include <stdlib.h>  // strtol()
#include <unistd.h>  // getopt()
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include "complex.h"

size_t num_threads = 1;
size_t num_lines = 2;
size_t num_items = 4; // num_lines^2
int d;                // simplification: assume degree is less than 10

double *roots;        // store the precomputed roots
double *re_inits;     // store the initial values of all the data points that are to be computed
double *im_inits;

char *categories;     // categorize each data point into the root it converges to
char *num_iters;      // store the number of iterations each computation needs to converge
char **color_sets;
char **gray_sets;

size_t max_iter = 90; // assume less than 100, but no less than 50
size_t len_color = 6; // assume degree less than 10, then the format of color should be "%d %d %d ", e.g. "3 4 5 "
size_t len_gray = 9;  // assume the max_iter less than 100 but no less than 50, the format "%d %d %d ", e.g. "58 58 58 "

pthread_mutex_t mutex_item;

void *newton(void *arg);
void *write_to_disc(void *arg);
void (*complex_power)(double *cpx);

int main (int argc, char **argv) {
    /* 
    Usage example: ./newton -t5 -l1000 7
      Inputs:
         The last argument is d, the exponent of x in f(x)=x^d-1 (required)
         -t gives the number of threads to be used (optinal, > 0, default 1)
           (besides the threads to compute the iteration,
            use one main thread for convenience,
            and one thread to write to disc)
         -l gives the number of rows and columns for the output picture (optinal, > 1, default 2)
    */

/* Bug with getopt(), optarg will be 0x0 and then segmentation fault if having two or more flags
 
    // use getopt to parse input parameters
    int c;
    int index;
    char *ptr;  // used in strtol(const char *str, char **endptr, int base)
    while ((c = getopt(argc, argv, "tl:")) != -1)
        switch (c) {
            case 'l' :
                num_lines = strtol(optarg, &ptr, 10);
                if (num_lines < 2) {
                    printf("number of lines must be greater than 1\n");
                    return 1;
                }
                num_items = num_lines * num_lines;
                break;
            case 't' :
                num_threads = strtol(optarg, &ptr, 10);
                if (num_threads < 1) {
                    printf("number of threads must be positive\n");
                    return 1;
                }
                break;
            default :      // unexpected option
                return 1;  // return error code, can be viewed in bash using " echo $? "
        }
    // deal with positional parameter
    if ((index = optind) == argc) {
        printf("no degree given\n");
        return 1;
    }
    for (int i = 0; index < argc; index++, i++) {
        if (i > 0)         // more than one number that indicates the exponent
            return 1;
        d = strtol(argv[index], &ptr, 10);
    }

*/
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

    // memory allocations, initializations
    
    roots = (double *) malloc(3 * d * sizeof(double));
    re_inits = (double *) malloc(num_lines * sizeof(double));
    im_inits = (double *) malloc(num_lines * sizeof(double));
    num_iters = (char *) malloc(num_items * sizeof(char));
    // initialize categories to zero. so if the entry is zero, means it hasn't been categorized yet
    categories = (char *) calloc(num_items, sizeof(char));

    // preset colors
    // we need d+1 kinds of color, each color has RGB value and corresponding spaces
    char *colors = (char *) malloc((d+1) * len_color * sizeof(char));
    // matrix form
    color_sets = (char **) malloc((d+1) * sizeof(char *));
    for (size_t i = 0; i < d+1; ++i)
        color_sets[i] = colors + i * len_color;
    // initialize colors
    for (size_t i = 0; i < d+1; ++i)
        for (int j = 0; j < len_color; j += 2) {
            color_sets[i][j] = (i+j) % (d+1) + '0';
            color_sets[i][j+1] = ' ';
        }

    // preset grays
    // according to valgrind memcheck, when using sprintf the string should add one more byte for '\0' as the end
    char *grays = (char *) malloc(max_iter * (len_gray+1) * sizeof(char));
    gray_sets = (char **) malloc(max_iter * sizeof(char *));
    for (size_t i = 0; i < max_iter; ++i)
        gray_sets[i] = grays + i * len_gray;
    // initialize grays
    for (size_t i = 0; i < max_iter; ++i)
        sprintf(gray_sets[i], "%02zu %02zu %02zu ", i, i, i);

    pthread_t newton_threads[num_threads];
    pthread_t writing_thread;
    size_t tx;
    int ret;

    // The computation is performed for complex number with real and imaginary part between -2 and +2,
    // the initial values are stored in shared memory that all threads can access to
    double interval = 4.0 / (num_lines - 1);
    double temp;
    for (size_t ix = 0; ix < num_lines; ++ix) {
        temp = -2 + interval * ix;
        re_inits[ix] = temp;
        im_inits[num_lines - 1 - ix] = temp;
    }

    // the pre-hardcoded complex power function
    switch(d-1) {
        case 0:
            complex_power = complex_power_0;
            break;
        case 1:
            complex_power = complex_power_1;
            break;
        case 2:
            complex_power = complex_power_2;
            break;
        case 3:
            complex_power = complex_power_3;
            break;
        case 4:
            complex_power = complex_power_4;
            break;
        case 5:
            complex_power = complex_power_5;
            break;
        case 6:
            complex_power = complex_power_6;
            break;
        case 7:
            complex_power = complex_power_7;
            break;
        case 8:
            complex_power = complex_power_8;
            break;
        //case 9:
        //    complex_power = complex_power_9;
        //    break;
        default:
            fprintf(stderr, "unexpected degree\n");
            exit(1);
    }

    // Precompute the exact values of the roots which can be used for comparison during the iteration later.
    // f(x) = x^d - 1, there should be d roots in total
    // the n_th root (n = 0,1,...,d-1) is x_n = re + im*i, where re = cos(2*pi*n/d), im = sin(2*pi*n/d)
    for (int n = 0; n < d; ++n) {
        //double re = cos(2 * M_PI * n / d);  // real part
        //double im = sin(2 * M_PI * n / d);  // imaginary part
        //roots[3*n  ] = re;
        //roots[3*n+1] = im;
        //roots[3*n+2] = 0.000001 - re*re - im*im;  // re^2 + im^2
        roots[2*n  ] = cos(2 * M_PI * n / d);
        roots[2*n+1] = sin(2 * M_PI * n / d);
    }
    
    // create threads
    for (tx = 0; tx < num_threads; ++tx) {
        size_t *arg = (size_t *) malloc(sizeof(size_t));
        *arg = tx;
        if (ret = pthread_create(&newton_threads[tx], NULL, newton, (void *)arg)) {
            // functions in pthread will return 0 if success
            printf("Error creating newton thread: %d\n", ret);
            exit(1);
        }
    }
    if (ret = pthread_create(&writing_thread, NULL, write_to_disc, NULL)) {
        printf("Error creating writing thread: %d\n", ret);
        exit(1);
    }

    for (tx = 0; tx < num_threads; ++tx) {
        if (ret = pthread_join(newton_threads[tx], NULL)) {
            printf("Error joining newton thread: %d\n", ret);
            exit(1);
        }
    }
    if (ret = pthread_join(writing_thread, NULL)) {
        printf("Error joining writing thread: %d\n", ret);
        exit(1);
    }

    pthread_mutex_destroy(&mutex_item);

    free(roots);
    free(re_inits); free(im_inits);
    free(categories);
    free(num_iters);
    free(colors); free(color_sets);
    free(grays); free(gray_sets);

    return 0;
}


void *newton (void *restrict arg) {
    // by making simplification that degree is less than 10,
    // it is allowed to hardcore formula for each degree case,
    // which is largely about finding an expression for the iteration step that is as efficient as possible.
    size_t offset = *((size_t *)arg);
    free(arg);
    
    double re_init, im_init;
    double re_next, im_next, re_prev, im_prev;
    double re_temp, im_temp, temp;
    double *cpx = (double *) malloc(2 * sizeof(double));
    size_t ix, iter;
    int n;
    char attr, conv;
    for (ix = offset; ix < num_items; ix += num_threads) {
        attr = d + 1;  // default, represents the additional root
        conv = max_iter - 1;  // default, represents unconvergence 
        // f(x) = x^d - 1, f'(x) = d * x^(d-1)
        re_init = re_inits[ix%num_lines];
        im_init = im_inits[ix/num_lines];
        // if x is zero, then f'(x) is zero, and can not be divided, thus categorizing to the additional root
        if (re_init == 0 && im_init == 0) {
            pthread_mutex_lock(&mutex_item);
            categories[ix] = attr;
            pthread_mutex_unlock(&mutex_item);
            num_iters[ix] = conv;
            continue;
        }

        re_prev = re_init;
        im_prev = im_init;
        for (iter = 0; iter < max_iter; ++iter) {
            // compute x^(d-1);
            cpx[0] = re_prev;
            cpx[1] = im_prev;
            complex_power(cpx);
            re_next = cpx[0];
            im_next = cpx[1];

            // x_next = x - f(x)/f'(x) = x - (x-1/x^(d-1))/d
            // 1/(a+bi) = (a-bi)/(a^2+b^2) = a*c-(b*c)i, c = 1/(a^2+b^2)
            temp = 1.0 / (re_next*re_next + im_next*im_next);
            re_next = re_prev - (re_prev - re_next * temp) / d;
            im_next = im_prev - (im_prev + im_next * temp) / d;

            // if x is closer than 10^-3 to one of the roots, abort iteration, categorize to the corresponding root
            // the label should be 1, 2, ..., d, and d+1 (additional root). 0 means the item has not done
            for (n = 0; n < d; ++n) {
                // temp = re_next * (re_next - 2*roots[3*n]) + im_next * (im_next - 2*roots[3*n+1]);
                re_temp = re_next - roots[2*n];
                im_temp = im_next - roots[2*n+1];
                temp = re_temp*re_temp + im_temp*im_temp;
                if (temp < 0.000001) { 
                    attr = n + 1;
                    conv = iter;
                    break;
                }
            }

            // if x is closer than 10^-3 to the origin, abort iteration, categorize to the additional root
            temp = re_next*re_next + im_next*im_next;
            // sqrt(d_re^2 + d_im^2) < 10^-3, is just d_re^2 + d_im^2 < 10^-6
            if (temp < 0.000001)
                break;
        
            // absolute value
            re_temp = (re_next < 0) ? (-re_next) : re_next;
            im_temp = (im_next < 0) ? (-im_next) : im_next;
            // if the absolute value of real or imaginary part is bigger than 10^10, abort iteration, categorize to the the additional root
            if (re_temp > 10000000000 || im_temp > 10000000000)
                break;

            if (attr != d+1)
                break;

            re_prev = re_next;
            im_prev = im_next;
        }
        pthread_mutex_lock(&mutex_item);
        categories[ix] = attr;
        pthread_mutex_unlock(&mutex_item);
        num_iters[ix] = conv;
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
    sleep_timespec.tv_nsec = 10000000;  // sleep 10ms
    
    char buffer[30];
    sprintf(buffer, "newton_attractors_x%d.ppm", d);
    FILE *fa = fopen(buffer, "w");
    sprintf(buffer, "newton_convergence_x%d.ppm", d);
    FILE *fc = fopen(buffer, "w");
    
    fprintf(fa, "P3\n%zu %zu\n%d\n", num_lines, num_lines, d);
    fprintf(fc, "P3\n%zu %zu\n%zu\n", num_lines, num_lines, max_iter);

    // according to valgrind memcheck, initialize the pointers
    char *color_buf = NULL;
    char *gray_buf = NULL;
    char *local = (char *) calloc(num_items, sizeof(char));
    for (size_t ix = 0; ix < num_items; ) {
        pthread_mutex_lock(&mutex_item);
        if (categories[ix] != 0)
            memcpy(local, categories, num_items * sizeof(char));
        pthread_mutex_unlock(&mutex_item);
        
        if (local[ix] == 0) {
            // sleep write_to_disc thread to avoiding locking the mutex all the time
            nanosleep(&sleep_timespec, NULL);
            continue;
        }

        for ( ; ix < num_items && local[ix] != 0; ++ix) {
            color_buf = color_sets[ local[ix]-1 ];
            gray_buf = gray_sets[ num_iters[ix] ];
            
            // write to attractors file
            fwrite(color_buf, sizeof(char), len_color, fa);
            // write to convergence file
            fwrite(gray_buf, sizeof(char), len_gray, fc);
            if ((ix+1) % num_lines == 0) {
                fwrite("\n", sizeof(char), 1, fa);
                fwrite("\n", sizeof(char), 1, fc);
            }
        }
    }
    free(local);
    fclose(fa);
    fclose(fc); 
}

