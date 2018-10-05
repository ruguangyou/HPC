#include <stdio.h>
#include <stdlib.h>  // strtol()
#include <unistd.h>  // getopt()
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <time.h>


int num_threads = 1;
int num_lines = 2;
int num_items = 4;   // num_lines^2
int d;

double *roots;       // store the precomputed roots

double *re_inits;    // store the initial values of all the data points that are to be computed
double *im_inits;

char *categories;    // categorize each data point into the root it converges to
int *num_iters;     // store the number of iterations each computation needs to converge
int max_iter = 100;

char **color_sets;
int len_color;

pthread_mutex_t mutex_item;

void *newton(void *arg);
void *write_to_disc(void *arg);

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
    char *endptr;
    if (argc < 2) {
        printf("no degree given\n");
        return 1;
    }
    else if (argc == 2) {
        d = strtol(argv[1], &endptr, 10); // if failed, will return 0
        if (d == 0) {
            printf("no degree given\n");
            return 1;
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
            printf("no degree given\n");
            return 1;
        }
        if (i_tl == 0) {
            printf("more than one degrees are given\n");
            return 1;
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
            printf("no degree given\n");
            return 1;
        }
        if (d > 13) {  // the degree should not be too large
            printf("unexpected degree\n");
            return 1;
        }
        if (i_t == 0 || i_l == 0) {
            printf("missing some parameter\n");
            return 1;
        }
        num_threads = strtol(argv[i_t]+2, &endptr, 10);
        num_lines = strtol(argv[i_l]+2, &endptr, 10);
    }
    else {
        printf("too much parameters\n");
        return 1;
    }

    if (num_threads < 1) {
        printf("number of threads must be positive\n");
        return 1;
    }
    if (num_lines < 2) {
        printf("number of lines must be greater than 1\n");
        return 1;
    }
    num_items = num_lines * num_lines;


    roots = (double *) malloc(2 * d * sizeof(double));
    re_inits = (double *) malloc(num_lines * sizeof(double));
    im_inits = (double *) malloc(num_lines * sizeof(double));
    num_iters = (int *) malloc(num_items * sizeof(int));
    // initialize categories to zero. so if the entry is zero, means it hasn't been categorized yet
    categories = (char *) calloc(num_items, sizeof(char));

    // preset colors
    int len_d = (d < 10) ? 1 : 2;  // we have set that the degree should not be larger than 13
    len_color = (len_d + 1) * 3;  
    // we need d+1 kinds of color, each color has RGB value and corresponding spaces
    char *colors = (char *) malloc((d+1) * len_color * sizeof(char));
    // matrix form
    color_sets = (char **) malloc((d+1) * sizeof(char *));
    for (int i = 0; i < d+1; ++i)
        color_sets[i] = colors + i * len_color;
    // initialize colors
    if (len_d == 1) {
        for (int i = 0; i < d+1; ++i)
            for (int j = 0; j < len_color; j += 2) {
                color_sets[i][j] = (i+j) % (d+1) + '0';
                color_sets[i][j+1] = ' ';
            }
    }
    else {
        for (int i = 0; i < d+1; ++i)
            for (int j = 0; j < len_color; j += 3) {
                color_sets[i][j] = (i + j) / d + '0';
                char c = (i+j) % (d+1);
                c = (c < 10) ? c : (c-10);
                color_sets[i][j+1] = c + '0';
                color_sets[i][j+2] = ' ';
            }
    }

    pthread_t newton_threads[num_threads];
    pthread_t writing_thread;
    int ret, tx;

    // The computation is performed for complex number with real and imaginary part between -2 and +2,
    // the initial values are stored in shared memory that all threads can access to
    double interval = 4.0 / (num_lines - 1);
    double temp;
    for (int ix = 0; ix < num_lines; ++ix) {
        temp = -2 + interval * ix;
        re_inits[ix] = temp;
        im_inits[num_lines - 1 - ix] = temp;
    }

    // Precompute the exact values of the roots which can be used for comparison during the iteration later.
    // f(x) = x^d - 1, there should be d roots in total
    // the n_th root (n = 0,1,...,d-1) is x_n = re + im*i, where re = cos(2*pi*n/d), im = sin(2*pi*n/d)
    for (int n = 0; n < d; ++n) {
        roots[2*n  ] = cos(2 * M_PI * n / d);  // real part
        roots[2*n+1] = sin(2 * M_PI * n / d);  // imaginary part
    }

    // create threads
    for (tx = 0; tx < num_threads; ++tx) {
        int *arg = (int *) malloc(sizeof(int));
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

    free(roots);
    free(re_inits); free(im_inits);
    free(categories);
    free(num_iters);
    free(colors); free(color_sets);

    return 0;
}

void *newton (void *restrict arg) {
    int offset = *((int *)arg);
    free(arg);
    
    double re_init, im_init;
    double re_next, im_next, re_prev, im_prev;
    double re_temp, im_temp, temp;
    int done;
    for (int ix = offset; ix < num_items; ix += num_threads) {
        // f(x) = x^d - 1, f'(x) = d * x^(d-1)
        re_init = re_inits[ix%num_lines];
        im_init = im_inits[ix/num_lines];
        // if x is zero, then f'(x) is zero, and can not be divided, thus categorizing to the additional root
        if (re_init == 0 && im_init == 0) {
            pthread_mutex_lock(&mutex_item);
            categories[ix] = d + 1;
            pthread_mutex_unlock(&mutex_item);
            num_iters[ix] = max_iter - 1;
            continue;
        }

        re_prev = re_init; im_prev = im_init;
        done = 0;
        for (int iter = 0; iter < max_iter; ++iter) {
            // compute x^(d-1);
            re_temp = 1; im_temp = 0;
            for (int i = 0; i < d-1; ++i) {
                re_next = re_temp * re_prev - im_temp * im_prev;
                im_next = re_temp * im_prev + im_temp * re_prev;
                re_temp = re_next;
                im_temp = im_next;
            }

            // 1/(a+bi) = (a-bi)/(a^2+b^2) = a/temp-(b/temp)i, temp = a^2+b^2
            temp = re_temp*re_temp + im_temp*im_temp;

            // x_next = x - f(x)/f'(x) = (1-1/d) * x + 1/d * 1/x^(d-1);
            re_next = (1.0 - 1.0/d) * re_prev + 1.0/d * re_temp/temp;
            im_next = (1.0 - 1.0/d) * im_prev - 1.0/d * im_temp/temp;

            // absolute value
            re_temp = (re_next < 0) ? (-re_next) : re_next;
            im_temp = (im_next < 0) ? (-im_next) : im_next;

            // if the absolute value of real or imaginary part is bigger than 10^10, abort iteration, categorize to the the additional root
            if (re_temp > 10000000000 || im_temp > 10000000000)
                break;

            // if x is closer than 10^-3 to the origin, abort iteration, categorize to the additional root
            re_temp = re_next - re_init;  // delta_re
            im_temp = im_next - im_init;  // delta_im
            // sqrt(d_re^2 + d_im^2) < 10^-3, is just d_re^2 + d_im^2 < 10^-6
            if (re_temp * re_temp + im_temp * im_temp < 0.000001)
                break;
        
            // if x is closer than 10^-3 to one of the roots, abort iteration, categorize to the corresponding root
            // the label should be 1, 2, ..., d, and d+1 (additional root). 0 means the item has not done
            for (int n = 0; n < d; ++n) {
                re_temp = re_next - roots[2*n  ];
                im_temp = im_next - roots[2*n+1];
                if (re_temp * re_temp + im_temp * im_temp < 0.000001) {
                    pthread_mutex_lock(&mutex_item);
                    categories[ix] = n + 1;
                    pthread_mutex_unlock(&mutex_item);
                    num_iters[ix] = iter;
                    done = 1;
                    break;
                }
            }
            if (done)
                break;

            re_prev = re_next;
            im_prev = im_next;
        }
        // if still not done, might be caused by conditions vialation, or reaching the max_iter
        if (!done) {
            pthread_mutex_lock(&mutex_item);
            categories[ix] = d + 1;
            pthread_mutex_unlock(&mutex_item);
            num_iters[ix] = max_iter - 1;
        }
    }
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
    sleep_timespec.tv_nsec = 1000000;
    
    char buffer[50];
    sprintf(buffer, "newton_accractors_x%d.ppm", d);
    FILE *attractors = fopen(buffer, "w");
    sprintf(buffer, "newton_convergence_x%d.ppm", d);
    FILE *convergence = fopen(buffer, "w");
    
    fprintf(attractors, "P3\n%d %d\n%d\n", num_lines, num_lines, d);
    fprintf(convergence, "P3\n%d %d\n%d\n", num_lines, num_lines, max_iter);

    char *color_buf;
    
    // get the number of digits of the max_iter
    int len = 0;
    int temp = max_iter;
    while (temp) {
        temp /= 10;
        len++;
    }
    char *iters_buf = (char *) malloc((len+1) * sizeof(char));
    int iters;
    
    char *local = (char *) calloc(num_items, sizeof(char));
    for (int ix = 0; ix < num_items; ) {
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
            if (ix > 0 && ix % (num_lines-1) == 0) {
                color_buf[len_color-1] = '\n';
                iters_buf[len] = '\n';
            }
            else {
                color_buf[len_color-1] = ' ';
                iters_buf[len] = ' ';
            }
            
            // write to attractors file
            fwrite(color_buf, 1, len_color, attractors);

            // write to convergence file
            iters = num_iters[ix];
            for (int i = len-1; i>= 0; --i) {
                iters_buf[i] = iters % 10 + '0';
                iters /= 10;
            }
            // RGB, write three times
            fwrite(iters_buf, 1, len+1, convergence);
            fwrite(iters_buf, 1, len+1, convergence);
            fwrite(iters_buf, 1, len+1, convergence);
        }
    }
    free(local);
    free(iters_buf);
    fclose(attractors);
    fclose(convergence); 
}

