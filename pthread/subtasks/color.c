#include <stdio.h>
#include <stdlib.h>  // strtol()

char **color_sets;
int len_color;

int main() {
    int d = 12;
    
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
                color_sets[i][j] = (i+j) / (d+1) + '0';
                char n = (i+j) % (d+1);
                n = (n < 10) ? n : (n-10); 
                color_sets[i][j+1] = n + '0';
                color_sets[i][j+2] = ' ';
            }
    }

    FILE *fp = fopen("color_sets.txt", "w");
    fwrite(colors, 1, (d+1) * len_color, fp);
    fclose(fp);

    return 0;
}
