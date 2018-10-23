#include <stdio.h>

int main() {
    FILE *fp;
    char buffer[50];
    int len;
    fp = fopen("text", "r");
    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    fread(buffer, 1, len+1, fp);
    fclose(fp);
    printf("len: %d\n", len);
    printf("%s\n", buffer);
}
