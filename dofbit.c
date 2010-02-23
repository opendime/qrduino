#include <string.h>

static unsigned char qrframe[4096];
static unsigned char framask[4096];
static unsigned char width, widbytes;

#define QRBIT(x,y) ( ( qrframe[((x)>>3) + (y) * widbytes] >> (7-((x) & 7 ))) & 1 )
#define SETQRBIT(x,y) qrframe[((x)>>3) + (y) * widbytes] |= 0x80 >> ((x) & 7)
#define SETFXBIT(x,y) framask[((x)>>3) + (y) * widbytes] |= 0x80 >> ((x) & 7)

#define TOGQRBIT(x,y) qrframe[((x)>>3) + (y) * widbytes] ^= 0x80 >> ((x) & 7)
#define FIXEDBIT(x,y) ( ( framask[((x)>>3) + (y) * widbytes] >> (7-((x) & 7 ))) & 1 )

static void putfind()
{
    unsigned char j, i, k, t;
    for (t = 0; t < 3; t++) {
        k = 0;
        i = 0;
        if (t == 1)
            k = (width - 7);
        if (t == 2)
            i = (width - 7);
        SETQRBIT(i + 3, k + 3);
        for (j = 0; j < 6; j++) {
            SETQRBIT(i + j, k);
            SETQRBIT(i, k + j + 1);
            SETQRBIT(i + 6, k + j);
            SETQRBIT(i + j + 1, k + 6);
        }
        // the outer wall makes this mask unnecessary

#ifndef QUICK
        for (j = 1; j < 5; j++) {
            SETFXBIT(i + j, k + 1);
            SETFXBIT(i + 1, k + j + 1);
            SETFXBIT(i + 5, k + j);
            SETFXBIT(i + j + 1, k + 5);
        }
#endif
        for (j = 2; j < 4; j++) {
            SETQRBIT(i + j, k + 2);
            SETQRBIT(i + 2, k + j + 1);
            SETQRBIT(i + 4, k + j);
            SETQRBIT(i + j + 1, k + 4);
        }
    }
}

static void putalign(int x, int y)
{
    int j;

    SETQRBIT(x, y);
    for (j = -2; j < 2; j++) {
        SETQRBIT(x + j, y - 2);
        SETQRBIT(x - 2, y + j + 1);
        SETQRBIT(x + 2, y + j);
        SETQRBIT(x + j + 1, y + 2);
    }
#ifndef QUICK
    for( j = 0 ; j < 2 ; j++ ) {
        SETFXBIT(x-1, y+j);
        SETFXBIT(x+1, y-j);
        SETFXBIT(x-j, y-1);
        SETFXBIT(x+j, y+1);
    }
#endif
}

static const unsigned char adelta[41] = {
    0, 11, 15, 19, 23, 27, 31,  // force 1 pat
    16, 18, 20, 22, 24, 26, 28, 20, 22, 24, 24, 26, 28, 28, 22, 24, 24,
    26, 26, 28, 28, 24, 24, 26, 26, 26, 28, 28, 24, 26, 26, 26, 28, 28,
};

void doaligns(unsigned char vers)
{
    unsigned char delta, x, y;
    if (vers < 2)
        return;
    delta = adelta[vers];
    y = width - 7;
    for (;;) {
        x = width - 7;
        while (x > delta - 3U) {
            putalign(x, y);
            if (x < delta)
                break;
            x -= delta;
        }
        if (y <= delta + 9U)
            break;
        y -= delta;
        putalign(6, y);
        putalign(y, 6);
    }
}

static const unsigned vpat[] = {
    0xc94, 0x5bc, 0xa99, 0x4d3, 0xbf6, 0x762, 0x847, 0x60d,
    0x928, 0xb78, 0x45d, 0xa17, 0x532, 0x9a6, 0x683, 0x8c9,
    0x7ec, 0xec4, 0x1e1, 0xfab, 0x08e, 0xc1a, 0x33f, 0xd75,
    0x250, 0x9d5, 0x6f0, 0x8ba, 0x79f, 0xb0b, 0x42e, 0xa64,
    0x541, 0xc69
};

static void putvpat(unsigned char vers)
{
    unsigned char x, y, bc;
    unsigned verinfo;
    if (vers < 7)
        return;
    verinfo = vpat[vers - 7];

    bc = 17;
    for (x = 0; x < 6; x++)
        for (y = 0; y < 3; y++, bc--)
            if (1&(bc > 11 ? vers >> (bc - 12) : verinfo >> bc)) {
                SETQRBIT( 5-x,2-y+width-11);
                SETQRBIT( 2-y+width-11,5-x);
            }
            else {
                SETFXBIT( 5-x,2-y+width-11);
                SETFXBIT( 2-y+width-11,5-x);
            }
}

void initframe(unsigned char vers)
{
    unsigned x, y;
    if (vers > 40)
        return;
    width = 17 + 4 * vers;
    widbytes = (width + 7) / 8;
    memset(qrframe, 0, sizeof(qrframe));
    memset(framask, 0, sizeof(framask));
    // finders
    putfind();
    // alignment blocks
    doaligns(vers);
    // single black
    SETQRBIT(8, width - 8);
    // timing gap - masks only
    for (y = 0; y < 7; y++) {
        SETFXBIT(7, y);
        SETFXBIT(width - 8, y);
        SETFXBIT(7, y + width - 7);
    }
    for (x = 0; x < 8; x++) {
        SETFXBIT(x, 7);
        SETFXBIT(x + width - 8, 7);
        SETFXBIT(x, width - 8);
    }
    // reserve mask-format area
    for (x = 0; x < 9; x++)
        SETFXBIT(x, 8);
    for (x = 0; x < 8; x++) {
        SETFXBIT(x + width - 8, 8);
        SETFXBIT(8, x);
    }
    for (y = 0; y < 7; y++)
        SETFXBIT(8, y + width - 7);
    // timing
    for (x = 0; x < width - 14; x++)
        if (x & 1) {
            SETFXBIT(8 + x, 6);
            SETFXBIT(6, 8 + x);
        } else {
            SETQRBIT(8 + x, 6);
            SETQRBIT(6, 8 + x);
        }

    // version block
    putvpat(vers);
    for (x = 0; x < width * widbytes; x++)
        framask[x] |= qrframe[x];
}

#include "ecctable.h"

#include <stdio.h>
#include <stdlib.h>
int main(int argc, char *argv[])
{
    unsigned char i, j, k, b;
    unsigned char v;   

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

    printf( "#ifndef __AVR__\n#define PROGMEM\n#define memcpy_P memcpy\n#define __LPM(x) *x\n#else\n"
            "#include <avr/pgmspace.h>\n#endif\nstatic const unsigned char framebase[] PROGMEM = {\n" );
    for (j = 0; j < v; j++) {
        for (i = 0; i < v; i+= 8) {
            b = 0;
            for( k = 0 ; k < 8 ; k++ ) {
                b <<= 1;
                if( i+k < v )
                    b |= QRBIT(i+k, j);
            }                    
            printf("0x%02x,",  b );
        }
        printf("\n");
    }
    printf( "};\n\nstatic const unsigned char framask[] PROGMEM = {\n" );
    for (j = 0; j < v; j++) {
        for (i = 0; i < v; i+= 8) {
            b = 0;
            for( k = 0 ; k < 8 ; k++ ) {
                b <<= 1;
                if( i+k < v )
                    b |= FIXEDBIT(i+k, j);
            }                    
            printf("0x%02x,",  b );
        }
        printf("\n");
    }
    printf( "};\n" );
    return 0;
}
