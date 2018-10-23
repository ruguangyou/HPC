//#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "time_diff.h"

#define LINES 3466  // 00.00 00.01 ... 34.65
#define ROW_LEN 24

int main (int argc, char **argv) {
    if (argc != 2 || argv[1][1] != 't') {
        fprintf(stdout, "Usage example: ./cell_distance -t2\n");
        return 1;
    }

    int ret;
    char *endptr;
    size_t n_threads, n_cells, n_dists, n_chars;
    size_t *dist_counts;
    short **cells;
    short *cell;
    char *buffer;
    double dx, dy, dz;
    short distance;
    size_t i, j, k;
    struct timespec start, stop, result;

    n_threads = strtol(argv[1]+2, &endptr, 10);
    //omp_set_num_threads(n_threads);

    dist_counts = (size_t *) calloc(LINES, sizeof(size_t));
    FILE *fp = fopen("cells", "r");
    fseek(fp, 0, SEEK_END);
    n_chars = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    n_cells = n_chars / ROW_LEN;
    cell = (short *) malloc(3 * n_cells * sizeof(short));
    cells = (short **) malloc(n_cells * sizeof(short *));
    for (int i = 0; i < n_cells; i++)
        cells[i] = cell + 3*i;

    buffer = (char *) malloc(n_chars * sizeof(char));
    ret = fread(buffer, n_chars, 1, fp);
    fclose(fp);

    timespec_get(&start, TIME_UTC);
    //#pragma omp parallel for private(i, j, k)
    for (i = 0; i < n_cells; ++i) {
        for (j = 0; j < 3; ++j) {
            k = i*24 + j*8;
            cells[i][j] =   (buffer[k+1]-'0')*10000
                          + (buffer[k+2]-'0')*1000
                          + (buffer[k+4]-'0')*100
                          + (buffer[k+5]-'0')*10
                          + (buffer[k+6]-'0');
            if (buffer[k] == '-')
                cells[i][j] *= -1;
        }
    }
    timespec_get(&stop, TIME_UTC);
    time_diff(&start, &stop, &result);
    printf("parsing time: %lf\n", (double)result.tv_nsec/1000000000 + result.tv_sec);
    
    n_dists = n_cells * (n_cells - 1) / 2;
    timespec_get(&start, TIME_UTC);
    //#pragma omp parallel for private(i, j, k, dx, dy, dz, dd, distance) reduction(+:dist_counts[:LINES])
    for (k = 0; k < n_dists; ++k) {
        i = k / n_cells + 1;
        j = k % n_cells;
        if (i <= j) {
            i = n_cells - i;
            j = n_cells - j - 1;
        }
        //printf("(%zu, %zu)\n", i, j);
        dx = cells[i][0] - cells[j][0];
        dy = cells[i][1] - cells[j][1];
        dz = cells[i][2] - cells[j][2];
        
        // This would be slower about 0.2s
        // dd = sqrt(dx*dx + dy*dy + dz*dz);
        // distance = ((int)dd%10 < 5) ? (int)dd/10 : (int)dd/10+1;
        
        distance = (short) (sqrt(dx*dx + dy*dy + dz*dz) / 10);
        
        // #pragma omp atomic
        ++dist_counts[distance];
    }
    timespec_get(&stop, TIME_UTC);
    time_diff(&start, &stop, &result);
    printf("computing time: %lf\n", (double)result.tv_nsec/1000000000 + result.tv_sec);

    //for (i = 0; i < LINES; i++)
        //fprintf(stdout, "%02zu.%02zu %zu\n", i/100, i%100, dist_counts[i]);

    free(dist_counts);
    free(cell); free(cells);
    free(buffer);
    return 0;
}
