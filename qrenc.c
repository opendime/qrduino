#include <stdio.h>
#include <stdlib.h>

extern char strinbuf[];
extern unsigned char *qrcode();

int main(int argc, char **argv)
{
    unsigned char *p;
    int x, y;
    int ret;

    ret = fread(strinbuf, 1, 136, stdin);
    strinbuf[ret] = '\0';

    p = qrcode();

    printf("P1\n%d %d\n", 41 + 8, 41 + 8);
    /* data */
    for (y = 0; y < 4; y++) {
        for (x = 0; x < 41 + 8; x++)
            printf("0 ");
        printf("\n");
    }
    for (y = 0; y < 41; y++) {
        for (x = 0; x < 4; x++)
            printf("0 ");
        for (x = 0; x < 41; x++)
            printf("%d ", 1 & *p++);
        for (x = 0; x < 4; x++)
            printf("0 ");
        printf("\n");
    }
    for (y = 0; y < 4; y++) {
        for (x = 0; x < 41 + 8; x++)
            printf("0 ");
        printf("\n");
    }

    return 0;
}
