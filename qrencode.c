#include <string.h>

#include "qrencode.h"

//========================================================================
// Reed Solomon error correction
static unsigned modnn(unsigned x)
{
    while (x >= 255) {
        x -= 255;
        x = (x >> 8) + (x & 255);
    }
    return x;
}

static void appendrs(unsigned char *data, unsigned char dsize, unsigned char *ecbuf, unsigned char ecsize)
{
#define GFPOLY 0x11d;
    unsigned i, j;
    unsigned char fb;
    // use qrframe as buffer space
    unsigned char *exp = qrframe, *log = &qrframe[256], *genpoly = &qrframe[512];

    memset(ecbuf, 0, ecsize);

    log[0] = 255;
    exp[255] = 0;
    j = 1;
    for (i = 0; i < 255; i++) {
        log[j] = i;
        exp[i] = j;
        j <<= 1;
        if (j & 256)
            j ^= GFPOLY;
        j &= 255;
    }

    genpoly[0] = 1;
    for (i = 0; i < ecsize; i++) {
        genpoly[i + 1] = 1;
        for (j = i; j > 0; j--) {
            if (genpoly[j])
                genpoly[j] = genpoly[j - 1] ^ exp[modnn(log[genpoly[j]] + i)];
            else
                genpoly[j] = genpoly[j - 1];
        }
        genpoly[0] = exp[modnn(log[genpoly[0]] + i)];
    }
    /* use logs for genpoly[]  */
    for (i = 0; i <= ecsize; i++)
        genpoly[i] = log[genpoly[i]];

    for (i = 0; i < dsize; i++) {
        fb = log[data[i] ^ ecbuf[0]];
        if (fb != 255)          /* fb term is non-zero */
            for (j = 1; j < ecsize; j++)
                ecbuf[j] ^= exp[modnn(fb + genpoly[ecsize - j])];
        /* Shift */
        memmove(&ecbuf[0], &ecbuf[1], ecsize - 1);
        if (fb != 255)
            ecbuf[ecsize - 1] = exp[modnn(fb + genpoly[0])];
        else
            ecbuf[ecsize - 1] = 0;
    }
}

//========================================================================
// 8 bit data to QR-coded 8 bit data
#include <stdio.h>
// 136 (-2), 2(68/18)
static void stringtoqr(void)
{
    unsigned char i;
    unsigned char size;
    size = strlen((char *) strinbuf);

    if (size > DATAWID*2-2)
        size = DATAWID*2-2;
    i = size;
    strinbuf[i + 1] = 0;
    while (i--) {
        strinbuf[i + 2] |= strinbuf[i] << 4;
        strinbuf[i + 1] = strinbuf[i] >> 4;
    }
    strinbuf[1] |= size << 4;
    strinbuf[0] = 0x40 | (size >> 4);
    i = size + 2;
    while (i < DATAWID*2) {
        strinbuf[i++] = 0xec;
        if (i == DATAWID*2)
            break;
        strinbuf[i++] = 0x11;
    }

    // Level tables
    // { 41,  172, 7, {  36,   64,   96,  112}},
    // 2,4,4,4
    // 18,16,24,28
    // split DATADATAxxxxxx to DATAeccDATAecc
    memmove(&strinbuf[DATAWID+ECCWID], &strinbuf[DATAWID], DATAWID);
    // calculate and append ECC
    appendrs(strinbuf, DATAWID, &strinbuf[DATAWID], ECCWID);
    appendrs(&strinbuf[DATAWID+ECCWID], DATAWID, &strinbuf[DATAWID*2+ECCWID],ECCWID);

}

//========================================================================
// Frame data insert following the path rules

static void fillframe(void)
{
    unsigned char d, i, j;
    unsigned char x, y, ffdecy, ffgohv;

    memcpy_P(qrframe, framebase, WDB * WD);
    //    printframe(qrframe);
    x = y = WD-1;
    ffdecy = 1;             // up, minus
    ffgohv = 1;

    /* inteleaved data and ecc codes */
    // as is, qrframe could be *c++, but I want to do bits
    for (i = 0; i < ((DATAWID+ECCWID)*2); i++) {

        d = strinbuf[(DATAWID+ECCWID) * (i & 1) + (i >> 1)];
        //        fprintf( stderr, "%02x", d );

        for (j = 0; j < 8; j++, d <<= 1) {

            if( 0x80 & d )
                SETQRBIT(x,y);
            //            fprintf( stderr, "%3d,%-3d\n", x,y );

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
                        if (y != WD-1)
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

            } while ( FIXEDBIT(x,y) );// (0x80 >> (x&7)) & __LPM(&framask[ y*6 + (x>>3) ] )); 
        }
    }
}

//========================================================================
// Masking 

static unsigned applymask(unsigned char m)
{
    unsigned char x, y, t = 0;
    int b = 0;

    for (y = 0; y < WD; y++)
        for (x = 0; x < WD; x++) {
            if (!FIXEDBIT(x,y)) { //((0x80 >> (x&7)) & __LPM(&framask[ y*6 + (x>>3) ]))) {
                switch (m) {
                case 0:
                    t = !((x + y) & 1);
                    break;
                case 1:
                    t = !(y & 1);
                    break;
                case 2:
                    t = !(x % 3);
                    break;
                case 3:
                    t = !((x + y) % 3);
                    break;
                case 4:
                    t = !(((y / 2) + (x / 3)) & 1);
                    break;
                case 5:
                    t = !(((x * y) & 1) + (x * y) % 3);
                    break;
                case 6:
                    t = !((((x * y) & 1) + (x * y) % 3) & 1);
                    break;
                case 7:
                    t = !((((x * y) % 3) + ((x + y) & 1)) & 1);
                    break;
                }
                if( t )
                    TOGQRBIT(x,y);
            }
            if (QRBIT(x,y))       // count excess whites v.s blacks
                b++;
            else
                b--;
        }
    if (b < 0)
        b = -b;
    return b;
}

// Badness coefficients.
static const unsigned char N1 = 3;
static const unsigned char N2 = 3;
static const unsigned char N3 = 40;
static const unsigned char N4 = 10;

static unsigned char rlens[WD + 1];
static unsigned badruns(unsigned char length)
{
    unsigned char i;
    unsigned runsbad = 0;
    for (i = 0; i <= length; i++)
        if (rlens[i] >= 5)
            runsbad += N1 + rlens[i] - 5;
    // BwBBBwB
    for (i = 3; i < length-1 ; i += 2)
        if (rlens[i - 2] == rlens[i + 2]
            && rlens[i + 2] == rlens[i - 1]
            && rlens[i - 1] == rlens[i + 1]
            && rlens[i - 1] * 3 == rlens[i] 
            // white around the black pattern?  Not part of spec
            && ( rlens[i - 3] == 0 // beginning
                 || i + 3 > length   // end
                 || rlens[i - 3] * 3 >= rlens[i] * 4
                 || rlens[i + 3] * 3 >= rlens[i] * 4 )
            )
            runsbad += N3;
    return runsbad;
}

static int badcheck()
{
    unsigned char x, y, h, b;
    unsigned thisbad = 0;

    // blocks of same color.

    for (y = 0; y < WD-1; y++)
        for (x = 0; x < WD-1; x++)
            if( ( QRBIT(x,y) && QRBIT(x+1,y) && QRBIT(x,y+1) && QRBIT(x+1,y+1) ) // all black
                || !( QRBIT(x,y) || QRBIT(x+1,y) || QRBIT(x,y+1) || QRBIT(x+1,y+1) ) ) // all white
                thisbad += N2;

    // X runs
    for (y = 0; y < WD; y++) {
        rlens[0] = 0;
        for (h = b = x = 0; x < WD; x++) {
            if (QRBIT(x,y) == b)
                rlens[h]++;
            else
                rlens[++h] = 1;
            b = QRBIT(x,y);
        }
        thisbad += badruns(h);
    }
    // Y runs
    for (x = 0; x < WD; x++) {
        rlens[0] = 0;
        for (h = b = y = 0; y < WD; y++) {
            if (QRBIT(x,y) == b)
                rlens[h]++;
            else
                rlens[++h] = 1;
            b = QRBIT(x,y);
        }
        thisbad += badruns(h);
    }
    return thisbad;
}

// final format bits with mask
// level << 3 | mask
static const unsigned fmtword[8] = { 
    0x77c4, 0x72f3, 0x7daa, 0x789d, 0x662f, 0x6318, 0x6c41, 0x6976, //L
#if 0
    0x5412, 0x5125, 0x5e7c, 0x5b4b, 0x45f9, 0x40ce, 0x4f97, 0x4aa0, //M
    0x355f, 0x3068, 0x3f31, 0x3a06, 0x24b4, 0x2183, 0x2eda, 0x2bed, //Q
    0x1689, 0x13be, 0x1ce7, 0x19d0, 0x0762, 0x0255, 0x0d0c, 0x083b, //H
#endif
};

static void addfmt(unsigned char masknum)
{
    unsigned fmtbits;
    unsigned char i, lvl = 0;

    fmtbits = fmtword[masknum + (lvl << 3)];
    // low byte
    for (i = 0; i < 8; i++, fmtbits >>= 1) 
        if( fmtbits & 1 )  {
            SETQRBIT(WD - 1 - i ,8);
            if (i < 6)
                SETQRBIT(8,i);
            else
                SETQRBIT(8,i+1);
        }
    // high byte
    for (i = 0; i < 7; i++, fmtbits >>= 1) 
        if( fmtbits & 1 ) {
            SETQRBIT(8,WD-7+i);
            if (i)
                SETQRBIT(6-i,8);
            else
                SETQRBIT(7,8);
        }
}

void qrencode()
{
    unsigned mindem = 30000;
    unsigned char best = 0;
    unsigned char i;
    unsigned badness;

    stringtoqr();

    for (i = 0; i < 8; i++) {
        fillframe();  // Inisde loop to avoid having separate mask buffer
        badness = applymask(i); // returns black-white imbalance
        badness *= 10;
        badness /= (WD * WD);
        badness *= N4;
        badness += badcheck();
#if 0 //ndef PUREBAD
        if (badness < WD*WD*5/4) {   // good enough - masks grow in compute complexity
            best = i;
            break;
        }
#endif
        fprintf( stderr, "%d\n", badness );
        if (badness < mindem) {
            mindem = badness;
            best = i;
        }
        if (best == 7)
            break;              // don't increment i to avoid redoing mask
    }
    if (best != i) {            // redo best mask - none good enough, last wasn't best
        fillframe();
        applymask(best);
    }
    addfmt(best);		// add in final format bytes
}
