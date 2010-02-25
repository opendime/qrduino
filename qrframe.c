#include <string.h>

extern unsigned char qrframe[];
extern unsigned char framask[];
extern unsigned char qrwidth, qrwidbytes;

#include "qrbits.h"

static void setmask(unsigned char x,unsigned char y)  {
    unsigned bt;
    if( x > y ) {
        bt = x;
        x = y;
        y = bt;
    }
    // y*y = 1+3+5...
    bt = y;
    bt *= y;
    bt += y;
    bt >>= 1;
    bt += x;
    framask[bt >> 3] |= 0x80 >> (bt & 7);
}

static void putfind()
{
    unsigned char j, i, k, t;
    for (t = 0; t < 3; t++) {
        k = 0;
        i = 0;
        if (t == 1)
            k = (qrwidth - 7);
        if (t == 2)
            i = (qrwidth - 7);
        SETQRBIT(i + 3, k + 3);
        for (j = 0; j < 6; j++) {
            SETQRBIT(i + j, k);
            SETQRBIT(i, k + j + 1);
            SETQRBIT(i + 6, k + j);
            SETQRBIT(i + j + 1, k + 6);
        }
        for (j = 1; j < 5; j++) {
            setmask(i + j, k + 1);
            setmask(i + 1, k + j + 1);
            setmask(i + 5, k + j);
            setmask(i + j + 1, k + 5);
        }
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
    for( j = 0 ; j < 2 ; j++ ) {
        setmask(x-1, y+j);
        setmask(x+1, y-j);
        setmask(x-j, y-1);
        setmask(x+j, y+1);
    }
}

static const unsigned char adelta[41] = {
    0, 11, 15, 19, 23, 27, 31,  // force 1 pat
    16, 18, 20, 22, 24, 26, 28, 20, 22, 24, 24, 26, 28, 28, 22, 24, 24,
    26, 26, 28, 28, 24, 24, 26, 26, 26, 28, 28, 24, 26, 26, 26, 28, 28,
};

static void doaligns(unsigned char vers)
{
    unsigned char delta, x, y;
    if (vers < 2)
        return;
    delta = adelta[vers];
    y = qrwidth - 7;
    for (;;) {
        x = qrwidth - 7;
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
                SETQRBIT( 5-x,2-y+qrwidth-11);
                SETQRBIT( 2-y+qrwidth-11,5-x);
            }
            else {
                setmask( 5-x,2-y+qrwidth-11);
                setmask( 2-y+qrwidth-11,5-x);
            }
}

void initframe(unsigned char vers)
{
    unsigned x, y;
    if (vers > 40)
        return;
    qrwidth = 17 + 4 * vers;
    qrwidbytes = (qrwidth + 7) / 8;
    memset(qrframe, 0, qrwidbytes*qrwidth);
    memset(framask, 0, qrwidbytes*qrwidth);
    // finders
    putfind();
    // alignment blocks
    doaligns(vers);
    // single black
    SETQRBIT(8, qrwidth - 8);
    // timing gap - masks only
    for (y = 0; y < 7; y++) {
        setmask(7, y);
        setmask(qrwidth - 8, y);
        setmask(7, y + qrwidth - 7);
    }
    for (x = 0; x < 8; x++) {
        setmask(x, 7);
        setmask(x + qrwidth - 8, 7);
        setmask(x, qrwidth - 8);
    }
    // reserve mask-format area
    for (x = 0; x < 9; x++)
        setmask(x, 8);
    for (x = 0; x < 8; x++) {
        setmask(x + qrwidth - 8, 8);
        setmask(8, x);
    }
    for (y = 0; y < 7; y++)
        setmask(8, y + qrwidth - 7);
    // timing
    for (x = 0; x < qrwidth - 14; x++)
        if (x & 1) {
            setmask(8 + x, 6);
            setmask(6, 8 + x);
        } else {
            SETQRBIT(8 + x, 6);
            SETQRBIT(6, 8 + x);
        }

    // version block
    putvpat(vers);
    for( y = 0 ; y < qrwidth; y++ )
        for (x = 0; x <= y; x++)
            if( QRBIT(x,y) )
                setmask(x,y);
}
