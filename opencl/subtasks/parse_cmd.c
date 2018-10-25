#include <stdio.h> 
#include <stdlib.h> 
#include "diffusion.h"

int main (int argc, char **argv) {
    if (argc != 6) {
        printf("Usage example: ./heat_diffusion 1000 100 -i1e10 -d0.02 -n20\n");
        return 1;
    }
    char *endptr;
    size_t width, height, iters;
    float init_value, diffusion_const;
    width = 0; height = 0;
    for (int i = 1; i < argc; ++i) {
        // strtol: string to long int
        // strtod: string to double
        if (argv[i][0] != '-') {
            if (width == 0)
                width = strtol(argv[i], &endptr, 10);
            else 
                height = strtol(argv[i], &endptr, 10);
        }
        else if (argv[i][1] == 'i') {
            init_value = strtod(argv[i]+2, &endptr);
        }
        else if (argv[i][1] == 'd') {
            diffusion_const = strtod(argv[i]+2, &endptr);
        }
        else if (argv[i][1] == 'n') {
            iters = strtol(argv[i]+2, &endptr, 10);
        }
        else {
            printf("unexpected argument\n");
            return 1;
        }
    }

    printf("width: %zu\nheight: %zu\ninit_value: %lf\ndiffusion_const: %lf\niters: %zu\n", 
        width, height, init_value, diffusion_const, iters);

    // initialize
    float *boxes = (float *) calloc(width*height, sizeof(float));
    int width_odd = width % 2;
    int height_odd =height % 2;
    size_t mid_width = width / 2;
    size_t mid_height = height / 2;
    if (width_odd && height_odd) {
        // both are odd numbers
        boxes[mid_height*width + mid_width] = init_value;
    }
    else if (width_odd && !height_odd) {
        // width is odd, height is even
        init_value /= 2;
        boxes[(mid_height-1)*width + mid_width] = init_value;
        boxes[mid_height*width + mid_width] = init_value;
    }
    else if (!width_odd && height_odd) {
        // width is even, height is odd
        init_value /= 2;
        boxes[mid_height*width + mid_width - 1] = init_value;
        boxes[mid_height*width + mid_width] = init_value;
    }
    else {
        // both are even numbers
        init_value /= 4;
        boxes[(mid_height-1)*width + mid_width - 1] = init_value;
        boxes[(mid_height-1)*width + mid_width] = init_value;
        boxes[mid_height*width + mid_width - 1] = init_value;
        boxes[mid_height*width + mid_width] = init_value;
    }

    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            printf("%.2lf ", boxes[h*width+w]);
        }
        printf("\n");
    }
    printf("\n");

    float *out = (float *) malloc(width*height*sizeof(float));
    diffusion(boxes, out, width, height, diffusion_const, iters);
    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            printf("%.2lf ", out[h*width+w]);
        }
        printf("\n");
    }

    free(boxes); free(out);
    return 0;
}