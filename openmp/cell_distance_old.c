#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define LINES 3466  // 00.00 00.01 ... 34.65
#define ROW_LEN 24
#define ROW_MAX_READ 10000
#define CHAR_MAX_READ 240000

int main (int argc, char **argv) {
    if (argc != 2 || argv[1][1] != 't') {
        fprintf(stdout, "Usage example: ./cell_distance -t2\n");
        return 1;
    }

    int ret;
    char *endptr;
    size_t n_threads, n_cells, n_chars, tmp_chars;
    size_t dist_counts[LINES];
    char buffer[CHAR_MAX_READ];
    short *cell;
    double dx, dy, dz;
    short distance;
    size_t i, j, k;

    n_threads = strtol(argv[1]+2, &endptr, 10);
    omp_set_num_threads(n_threads);

    for (i = 0; i < LINES; ++i)
        dist_counts[i] = 0;

    FILE *fp = fopen("cells", "r");
    fseek(fp, 0, SEEK_END);
    n_chars = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    n_cells = n_chars / ROW_LEN;
    cell = (short *) malloc(3 * n_cells * sizeof(short));

    tmp_chars = CHAR_MAX_READ;
    if (n_chars < CHAR_MAX_READ)
        tmp_chars = n_chars;
    ret = fread(buffer, tmp_chars, 1, fp);
    fclose(fp);

    #pragma omp parallel for private(i, j, k)
    for (i = 0; i < n_cells; ++i) {
        for (j = 0; j < 3; ++j) {
            k = i*24 + j*8;
            cell[3*i+j] =   (buffer[k+1]-'0')*10000
                          + (buffer[k+2]-'0')*1000
                          + (buffer[k+4]-'0')*100
                          + (buffer[k+5]-'0')*10
                          + (buffer[k+6]-'0');
            if (buffer[k] == '-')
                cell[3*i+j] *= -1;
        }
    }
    
    #pragma omp parallel for private(i, j, k, dx, dy, dz, distance) reduction(+:dist_counts[:LINES])
    for (i = 0; i < n_cells; ++i) {
        for (j = i+1; j < n_cells; ++j) {
            dx = (cell[3*i  ] - cell[3*j  ]);
            dy = (cell[3*i+1] - cell[3*j+1]);
            dz = (cell[3*i+2] - cell[3*j+2]);
            distance = (short) (sqrt(dx*dx + dy*dy + dz*dz) / 10);
            ++dist_counts[distance];
        }
    }
    // for (k = 0; k < n_dists; ++k) {
    //     int square;
    //     i = k / n_cells + 1;
    //     j = k % n_cells;
    //     if (i <= j) {
    //         i = n_cells - i;
    //         j = n_cells - j - 1;
    //     }
    //     dx = (cell[3*i  ] - cell[3*j  ]);
    //     dy = (cell[3*i+1] - cell[3*j+1]);
    //     dz = (cell[3*i+2] - cell[3*j+2]);
    //     distance = (short) (sqrt(dx*dx + dy*dy + dz*dz) / 10);
    //     #pragma omp atomic
    //     ++dist_counts[distance];
    // }

    for (i = 0; i < LINES; i++)
        if (dist_counts[i] != 0)
        fprintf(stdout, "%02zu.%02zu %zu\n", i/100, i%100, dist_counts[i]);

    free(cell);
    return 0;
}
