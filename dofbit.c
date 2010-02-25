#include <string.h>

unsigned char qrframe[177*177];
unsigned char framask[177*89];
unsigned char qrwidth, qrwidbytes;

#include "qrbits.h"

void initframe(unsigned char vers);


#include "ecctable.h"

#include <stdio.h>
#include <stdlib.h>
int main(int argc, char *argv[])
{
    unsigned char i, j, b;
    unsigned char v,w;   

    if( argc != 3 )
        printf( "ruires Version ECC-level (1-4)" );

    unsigned ecc = atoi(argv[2]);
    if( ecc < 1 || ecc > 4 )
        return -1;
    unsigned char vers = atoi(argv[1]);
    if( vers > 40 ) 
        return -1;

    printf( "#define VERSION (%d)\n", vers );
    printf( "#define ECCLEVEL (%d)\n", ecc );

    ecc -= 1;
    ecc *= 4;
    ecc += (vers - 1) * 16;

    printf( "#define BLOCKS1 (%d)\n", eccblocks[ecc++] );
    printf( "#define BLOCKS2 (%d)\n", eccblocks[ecc++] );
    printf( "#define DATAWID (%d)\n", eccblocks[ecc++] );
    printf( "#define ECCWID (%d)\n", eccblocks[ecc++] );

    initframe(vers);
    v = vers * 4 + 17;

    printf( "#define WD (%d)\n#define WDB ((WD+7)/8)\n", v ); // width
    printf( "static const unsigned char qrwidbytes = ((WD+7)/8);\n" );

    printf( "#ifndef __AVR__\n#define PROGMEM\n#define memcpy_P memcpy\n#define __LPM(x) *x\n#else\n"
            "#include <avr/pgmspace.h>\n#endif\nstatic const unsigned char framebase[] PROGMEM = {\n" );

    w = (v+7)>>3;
    for (j = 0; j < v; j++) {
        for (i = 0; i < v; i+= 8) {
            b = qrframe[j*w+(i>>3)];
            printf("0x%02x,",  b );
        }
        printf("\n");
    }
    printf( "};\n\nstatic const unsigned char framask[] PROGMEM = {\n" );
    unsigned tt, tri = v*(v+1)/2;
    tri = (tri+7)/8;
    for( tt = 0; tt < tri; tt++ ) {
        if( !(tt % w) )
            printf("\n");
        printf("0x%02x,",  framask[tt] );
    }
    printf( "\n};\n" );
#if 0
    for (j = 0; j < v; j++) {
        for (i = 0; i < v; i++) {
            printf(" %d",  FIXEDBIT(j,i) );
        }
        printf("\n");
    }
#endif

    return 0;
}
