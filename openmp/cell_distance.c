#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define LINES 3466  // 00.00 00.01 ... 34.65
#define ROW_MAX_READ 10000
#define CHAR_MAX_READ 240000

int main (int argc, char **argv) {
    if (argc != 2 || argv[1][1] != 't') {
        fprintf(stdout, "Usage example: ./cell_distance -t2\n");
        return 1;
    }

    int ret;
    char *endptr;
    size_t n_threads, n_cells0, n_cells1, n_chars, n_segs, tmp_chars;
    double dx, dy, dz;
    size_t i, j, k;
    short distance;
    size_t dist_counts[LINES];
    char buffer0[CHAR_MAX_READ];
    char buffer1[CHAR_MAX_READ];
    short cells0[3*ROW_MAX_READ];
    short cells1[3*ROW_MAX_READ];

    n_threads = strtol(argv[1]+2, &endptr, 10);
    omp_set_num_threads(n_threads);

    for (i = 0; i < LINES; ++i)
        dist_counts[i] = 0;

    FILE *fp = fopen("cells", "r");
    fseek(fp, 0, SEEK_END);
    n_chars = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    n_segs = n_chars / CHAR_MAX_READ;
    if (n_chars % CHAR_MAX_READ != 0)
        n_segs++;

    if (n_segs == 1) {
        ret = fread(buffer0, n_chars, 1, fp);
        n_cells0 = n_chars / 24;

        #pragma omp parallel for collapse(2) private(i, j, k)
        for (i = 0; i < n_cells0; ++i)
            for (j = 0; j < 3; ++j) {
                k = i*24 + j*8;
                cells0[3*i+j] =   (buffer0[k+1]-'0')*10000
                                + (buffer0[k+2]-'0')*1000
                                + (buffer0[k+4]-'0')*100
                                + (buffer0[k+5]-'0')*10
                                + (buffer0[k+6]-'0');
                if (buffer0[k] == '-')
                    cells0[3*i+j] *= -1;
            }
        #pragma omp parallel for private(i, j, k, dx, dy, dz, distance) reduction(+:dist_counts[:LINES])
        for (i = 0; i < n_cells0; ++i) 
            for (j = i+1; j < n_cells0; ++j) {
                dx = (cells0[3*i  ] - cells0[3*j  ]);
                dy = (cells0[3*i+1] - cells0[3*j+1]);
                dz = (cells0[3*i+2] - cells0[3*j+2]);
                distance = (short) (sqrt(dx*dx + dy*dy + dz*dz) / 10);
                ++dist_counts[distance];
            }
    }
    else {
        for (int ix = 0; ix < n_segs; ++ix) {
            fseek(fp, ix*CHAR_MAX_READ, SEEK_SET);
            ret = fread(buffer0, CHAR_MAX_READ, 1, fp);
            n_cells0 = ROW_MAX_READ;

            #pragma omp parallel for collapse(2) private(i, j, k)
            for (i = 0; i < n_cells0; ++i) {
                for (j = 0; j < 3; ++j) {
                    k = i*24 + j*8;
                    cells0[3*i+j] =   (buffer0[k+1]-'0')*10000
                                    + (buffer0[k+2]-'0')*1000
                                    + (buffer0[k+4]-'0')*100
                                    + (buffer0[k+5]-'0')*10
                                    + (buffer0[k+6]-'0');
                    if (buffer0[k] == '-')
                        cells0[3*i+j] *= -1;
                }
            }
            #pragma omp parallel for private(i, j, k, dx, dy, dz, distance) reduction(+:dist_counts[:LINES])
            for (i = 0; i < n_cells0; ++i)
                for (j = i+1; j < n_cells0; ++j) {
                    dx = (cells0[3*i  ] - cells0[3*j  ]);
                    dy = (cells0[3*i+1] - cells0[3*j+1]);
                    dz = (cells0[3*i+2] - cells0[3*j+2]);
                    distance = (short) (sqrt(dx*dx + dy*dy + dz*dz) / 10);
                    ++dist_counts[distance];
                }

            for (int jx = ix+1; jx < n_segs; ++jx) {
                tmp_chars = CHAR_MAX_READ;
                n_cells1 = ROW_MAX_READ;
                if (jx == n_segs-1 && n_chars % CHAR_MAX_READ != 0) {
                    tmp_chars = n_chars % CHAR_MAX_READ;
                    n_cells1 = tmp_chars / 24;
                }
                ret = fread(buffer1, tmp_chars, 1, fp);
                
                #pragma omp parallel for collapse(2) private(i, j, k)
                for (i = 0; i < n_cells1; ++i) 
                    for (j = 0; j < 3; ++j) {
                        k = i*24 + j*8;
                        cells1[3*i+j] =   (buffer1[k+1]-'0')*10000
                                        + (buffer1[k+2]-'0')*1000
                                        + (buffer1[k+4]-'0')*100
                                        + (buffer1[k+5]-'0')*10
                                        + (buffer1[k+6]-'0');
                        if (buffer1[k] == '-')
                            cells1[3*i+j] *= -1;
                    }
                #pragma omp parallel for private(i, j, k, dx, dy, dz, distance) reduction(+:dist_counts[:LINES])
                for (i = 0; i < n_cells0; ++i) 
                    for (j = 0; j < n_cells1; ++j) {
                        dx = (cells0[3*i  ] - cells1[3*j  ]);
                        dy = (cells0[3*i+1] - cells1[3*j+1]);
                        dz = (cells0[3*i+2] - cells1[3*j+2]);
                        distance = (short) (sqrt(dx*dx + dy*dy + dz*dz) / 10);
                        ++dist_counts[distance];
                    }
            }
        }
    }

    for (i = 0; i < LINES; i++)
        //if (dist_counts[i] != 0)
        fprintf(stdout, "%02zu.%02zu %zu\n", i/100, i%100, dist_counts[i]);

    fclose(fp);
    return 0;
}
