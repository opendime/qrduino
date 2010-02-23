#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "qrencode.h"

int main(int argc, char **argv)
{
    int x, y;
    int ret;

    ret = fread(strinbuf, 1, 136, stdin);
    strinbuf[ret] = '\0';

    //    strcpy((char *)strinbuf, "Hello World!");

    qrencode();

    printf("P1\n%d %d\n", WD + 8, WD + 8);
    /* data */
    for (y = 0; y < 4; y++) {
        for (x = 0; x < WD + 8; x++)
            printf("0 ");
        printf("\n");
    }
    for (y = 0; y < WD; y++) {
        for (x = 0; x < 4; x++)
            printf("0 ");
        for (x = 0; x < WD; x++)
            printf("%d ", QRBIT(x,y) );
        for (x = 0; x < 4; x++)
            printf("0 ");
        printf("\n");
    }
    for (y = 0; y < 4; y++) {
        for (x = 0; x < WD + 8; x++)
            printf("0 ");
        printf("\n");
    }

    return 0;
}
