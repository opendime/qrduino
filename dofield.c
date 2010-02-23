#include <string.h>

static unsigned char qrframe[177 * 177];
static unsigned char width;

static void putfind()
{
    unsigned char *p;
    unsigned char j;
    p = qrframe;
    for (j = 0; j < 7; j++) {
        memset(p, 0xc1, 7);
        p += width;
    }
    p -= 6 * width - 1;
    memset(p,0xc0,5);
    for (j = 0; j < 3; j++) {
        p += width;
        *p = 0xc0;
        p[4] = 0xc0;
    }
    p += width;
    memset(p,0xc0,5);
    p = qrframe;
    for (j = 0; j < 7; j++) {
        memcpy(&p[width*(width-7)],p,7);
        memcpy(&p[width-7],p,7);
        p += width;
    }
}

static const unsigned char midalg[] = { 0xa1, 0xa0, 0xa0, 0xa0, 0xa1 };
static const unsigned char ctralg[] = { 0xa1, 0xa0, 0xa1, 0xa0, 0xa1 };
static void putalign(int x, int y)
{
    unsigned char *p = qrframe;
    p += (y - 2) * width + x - 2;
    memset(p, 0xa1, 5);
    memcpy(&p[width*1], midalg, 5);
    memcpy(&p[width*2], ctralg, 5);
    memcpy(&p[width*3], midalg, 5);
    memset(&p[width*4], 0xa1, 5);
}

static const unsigned char adelta[41] = { 
    0,11,15,19,23,27,31, // force 1 pat
    16,18,20,22,24,26,28,20,22,24,24,26,28,28,22,24,24,
    26,26,28,28,24,24,26,26,26,28,28,24,26,26,26,28,28,
};

void doaligns(unsigned char vers)
{
    unsigned char delta, x, y;
    if( vers < 2 )
        return;
    delta = adelta[vers];
    y = width - 7;
    for(;;) {
        x = width - 7;
        while (x > delta - 3U) {
            putalign(x, y);
            if( x < delta )
                break;
            x -= delta;
        }
        if( y <= delta + 9U )
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
    unsigned char x, y, *p, bc;
    unsigned verinfo;
    if (vers < 7)
        return;
    verinfo = vpat[vers - 7];

    p = qrframe + width * (width - 11);
    bc = 17;
    for (x = 0; x < 6; x++)
        for (y = 0; y < 3; y++, bc--) 
            p[width * y + x] = 0x88 | (1 & (bc > 11 ? vers >> (bc-12) : verinfo >> bc));

    p = qrframe + width - 11;
    bc = 17;
    for (y = 0; y < 6; y++) {
        for (x = 0; x < 3; x++, bc--)
            p[x] = 0x88 | (1 & (bc > 11 ? vers >> (bc-12) : verinfo >> bc));
        p += width;
    }
}

void initframe(unsigned char vers)
{
    unsigned char *p, *q;
    int x, y;
    if (vers > 40)
        return;
    width = 17 + 4 * vers;
    memset(qrframe, 0, width * width);
    // finders
    putfind();
    // timing gap
    p = qrframe;
    q = qrframe + width * (width - 7);
    for (y = 0; y < 7; y++) {
        p[7] = 0xc0;
        p[width - 8] = 0xc0;
        q[7] = 0xc0;
        p += width;
        q += width;
    }
    memset(qrframe + width * 7, 0xc0, 8);
    memset(qrframe + width * 8 - 8, 0xc0, 8);
    memset(qrframe + width * (width - 8), 0xc0, 8);
    // reserve mask-format area
    memset(qrframe + width * 8, 0x84, 9);
    memset(qrframe + width * 9 - 8, 0x84, 8);
    p = qrframe + 8;
    for (y = 0; y < 8; y++) {
        *p = 0x84;
        p += width;
    }
    p = qrframe + width * (width - 7) + 8;
    for (y = 0; y < 7; y++) {
        *p = 0x84;
        p += width;
    }
    // timing
    p = qrframe + width * 6 + 8;
    q = qrframe + width * 8 + 6;
    for (x = 1; x < width - 15; x++) {
        *p = 0x90 | (x & 1);
        *q = 0x90 | (x & 1);
        p++;
        q += width;
    }
    // alignment blocks
    doaligns(vers);
    // version block
    putvpat(vers);
    // single black
    qrframe[width * (width - 8) + 8] = 0x81;
}


//========================================================================
// Frame data insert following the path rules
//NOTTEST

#define BLOCKSPAN 86
static void fillframe(unsigned char *strinbuf, unsigned length)
{
    unsigned char d, i, j;
    unsigned char x, y, ffdecy, ffgohv;

    x = y = width-1;
    ffdecy = 1;             // up, minus
    ffgohv = 1;

    /* inteleaved data and ecc codes */
    // as is, qrframe could be *c++, but I want to do bits
    for (i = 0; i < length; i++) {


        d = strinbuf[BLOCKSPAN * (i & 1) + (i >> 1)];


        for (j = 0; j < 8; j++, d <<= 1) {

            qrframe[y * width + x] = !!(0x80 & d);

            do { // find next fill position
                if (ffgohv)
                    x--;
                else {
                    x++;
                    if (ffdecy) {
                        if (y != 0)
                            y--;
                        else {
                            x -= 2;
                            ffdecy = !ffdecy;
                            if (x == 6) {
                                x--;
                                y = 9;
                            }
                        }
                    } else {
                        if (y != 40)
                            y++;
                        else {
                            x -= 2;
                            ffdecy = !ffdecy;
                            if (x == 6) {
                                x--;
                                y -= 8;
                            }
                        }
                    }
                }
                ffgohv = !ffgohv;

            } while ( 0x80 & qrframe[y * width + x] ); 
        }
    }
}

#include <stdio.h>
#include <stdlib.h>
int main(int argc, char *argv[])
{
    int i, j;
    int v;// = atoi(argv[1]);
    int vers;
    for( vers = 1 ; vers < 41; vers++ ) {
        initframe(vers);
        v = vers * 4 + 17;
        for (j = 0; j < v; j++) {
            for (i = 0; i < v; i++)
                printf("%c", qrframe[j * v + i] & 1 ? '#': (qrframe[j * v + i] & 0x80 ? 'o':'.') );
            printf("\n");
        }
    }
    return 0;
}
