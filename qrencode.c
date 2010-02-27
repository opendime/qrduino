#include <string.h>

#include "qrencode.h"

extern unsigned char neccblk1;
extern unsigned char neccblk2;
extern unsigned char datablkw;
extern unsigned char eccblkwid;
extern unsigned char VERSION;
extern unsigned char ECCLEVEL;
extern unsigned char WD, WDB;
#ifndef USEPRECALC
// These are malloced by initframe
extern unsigned char *rlens;
extern unsigned char *framebase;
extern unsigned char *framask;
#else
extern unsigned char rlens[];
extern unsigned char framebase[] PROGMEM;
extern unsigned char framask[] PROGMEM;
#endif

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
    unsigned char *exp = qrframe, *log = &qrframe[256], *genpoly = &qrframe[512], *iecbuf;

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

    memset(ecbuf, 0, ecsize);
    iecbuf = ecbuf;
    for (i = 0; i < dsize; i++) {
        fb = log[data[i] ^ iecbuf[0]];
        if (fb != 255)          /* fb term is non-zero */
            for (j = 1; j < ecsize; j++)
                iecbuf[j] ^= exp[modnn(fb + genpoly[ecsize - j])];
        // Shift - use bigger buffer and move pointer instead of:
        // memmove(&iecbuf[0], &iecbuf[1], ecsize - 1);
        iecbuf++;
        if (fb != 255)
            iecbuf[ecsize - 1] = exp[modnn(fb + genpoly[0])];
        else
            iecbuf[ecsize - 1] = 0;
    }
    memmove(ecbuf, iecbuf, ecsize);
}

//========================================================================
// 8 bit data to QR-coded 8 bit data
static void stringtoqr(void)
{
    unsigned i;
    unsigned size, max;
    size = strlen((char *) strinbuf);

    max = datablkw * (neccblk1 + neccblk2) + neccblk2;
    if (size >= max - 2) {
        size = max - 2;
        if (VERSION > 9)
            size--;
    }

    i = size;
    if (VERSION > 9) {
        strinbuf[i + 2] = 0;
        while (i--) {
            strinbuf[i + 3] |= strinbuf[i] << 4;
            strinbuf[i + 2] = strinbuf[i] >> 4;
        }
        strinbuf[2] |= size << 4;
        strinbuf[1] = size >> 4;
        strinbuf[0] = 0x40 | (size >> 12);
    } else {
        strinbuf[i + 1] = 0;
        while (i--) {
            strinbuf[i + 2] |= strinbuf[i] << 4;
            strinbuf[i + 1] = strinbuf[i] >> 4;
        }
        strinbuf[1] |= size << 4;
        strinbuf[0] = 0x40 | (size >> 4);
    }
    i = size + 3 - (VERSION < 10);
    while (i < max) {
        strinbuf[i++] = 0xec;
        // buffer has room        if (i == max)            break;
        strinbuf[i++] = 0x11;
    }

    // calculate and append ECC
    unsigned char *ecc = &strinbuf[max];
    unsigned char *dat = strinbuf;
    for (i = 0; i < neccblk1; i++) {
        appendrs(dat, datablkw, ecc, eccblkwid);
        dat += datablkw;
        ecc += eccblkwid;
    }
    for (i = 0; i < neccblk2; i++) {
        appendrs(dat, datablkw + 1, ecc, eccblkwid);
        dat += datablkw + 1;
        ecc += eccblkwid;
    }
    unsigned j;
    dat = qrframe;
    for (i = 0; i < datablkw; i++) {
        for (j = 0; j < neccblk1; j++)
            *dat++ = strinbuf[i + j * datablkw];
        for (j = 0; j < neccblk2; j++)
            *dat++ = strinbuf[(neccblk1 * datablkw) + i + (j * (datablkw + 1))];
    }
    for (j = 0; j < neccblk2; j++)
        *dat++ = strinbuf[(neccblk1 * datablkw) + i + (j * (datablkw + 1))];
    for (i = 0; i < eccblkwid; i++)
        for (j = 0; j < neccblk1 + neccblk2; j++)
            *dat++ = strinbuf[max + i + j * eccblkwid];
    memcpy(strinbuf, qrframe, max + eccblkwid * (neccblk1 + neccblk2));

}

//========================================================================
// Frame data insert following the path rules
static unsigned char ismasked(unsigned char x, unsigned char y)
{
    unsigned bt;
    if (x > y) {
        bt = x;
        x = y;
        y = bt;
    }
    bt = y;
    bt += y * y;
#if 0
    // bt += y*y;
    unsigned s = 1;
    while (y--) {
        bt += s;
        s += 2;
    }
#endif
    bt >>= 1;
    bt += x;
    return (__LPM(&framask[bt >> 3]) >> (7 - (bt & 7))) & 1;
}

static void fillframe(void)
{
    unsigned i;
    unsigned char d, j;
    unsigned char x, y, ffdecy, ffgohv;

    memcpy_P(qrframe, framebase, WDB * WD);
    x = y = WD - 1;
    ffdecy = 1;                 // up, minus
    ffgohv = 1;

    /* inteleaved data and ecc codes */
    for (i = 0; i < ((datablkw + eccblkwid) * (neccblk1 + neccblk2) + neccblk2); i++) {
        d = strinbuf[i];
        for (j = 0; j < 8; j++, d <<= 1) {
            if (0x80 & d)
                SETQRBIT(x, y);
            do {                // find next fill position
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
                        if (y != WD - 1)
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
            } while (ismasked(x, y));
        }
    }

}

//========================================================================
// Masking 
static void applymask(unsigned char m)
{
    unsigned char x, y, r3x, r3y;

    switch (m) {
    case 0:
        for (y = 0; y < WD; y++)
            for (x = 0; x < WD; x++)
                if (!((x + y) & 1) && !ismasked(x, y))
                    TOGQRBIT(x, y);
        break;
    case 1:
        for (y = 0; y < WD; y++)
            for (x = 0; x < WD; x++)
                if (!(y & 1) && !ismasked(x, y))
                    TOGQRBIT(x, y);
        break;
    case 2:
        for (y = 0; y < WD; y++)
            for (r3x = 0, x = 0; x < WD; x++, r3x++) {
                if (r3x == 3)
                    r3x = 0;
                if (!r3x && !ismasked(x, y))
                    TOGQRBIT(x, y);
            }
        break;
    case 3:
        for (r3y = 0, y = 0; y < WD; y++, r3y++) {
            if (r3y == 3)
                r3y = 0;
            for (r3x = r3y, x = 0; x < WD; x++, r3x++) {
                if (r3x == 3)
                    r3x = 0;
                if (!r3x && !ismasked(x, y))
                    TOGQRBIT(x, y);
            }
        }
        break;
    case 4:
        for (y = 0; y < WD; y++)
            for (r3x = 0, r3y = ((y >> 1) & 1), x = 0; x < WD; x++, r3x++) {
                if (r3x == 3) {
                    r3x = 0;
                    r3y = !r3y;
                }
                if (!r3y && !ismasked(x, y))
                    TOGQRBIT(x, y);
            }
        break;
    case 5:
        for (r3y = 0, y = 0; y < WD; y++, r3y++) {
            if (r3y == 3)
                r3y = 0;
            for (r3x = 0, x = 0; x < WD; x++, r3x++) {
                if (r3x == 3)
                    r3x = 0;
                if (!((x & y & 1) + !(!r3x | !r3y)) && !ismasked(x, y))
                    TOGQRBIT(x, y);
            }
        }
        break;
    case 6:
        for (r3y = 0, y = 0; y < WD; y++, r3y++) {
            if (r3y == 3)
                r3y = 0;
            for (r3x = 0, x = 0; x < WD; x++, r3x++) {
                if (r3x == 3)
                    r3x = 0;
                if (!(((x & y & 1) + (r3x && (r3x == r3y))) & 1) && !ismasked(x, y))
                    TOGQRBIT(x, y);
            }
        }
        break;
    case 7:
        for (r3y = 0, y = 0; y < WD; y++, r3y++) {
            if (r3y == 3)
                r3y = 0;
            for (r3x = 0, x = 0; x < WD; x++, r3x++) {
                if (r3x == 3)
                    r3x = 0;
                if (!(((r3x && (r3x == r3y)) + ((x + y) & 1)) & 1) && !ismasked(x, y))
                    TOGQRBIT(x, y);
            }
        }
        break;
    }
    return;
}

// Badness coefficients.
static const unsigned char N1 = 3;
static const unsigned char N2 = 3;
static const unsigned char N3 = 40;
static const unsigned char N4 = 10;

static unsigned badruns(unsigned char length)
{
    unsigned char i;
    unsigned runsbad = 0;
    for (i = 0; i <= length; i++)
        if (rlens[i] >= 5)
            runsbad += N1 + rlens[i] - 5;
    // BwBBBwB
    for (i = 3; i < length - 1; i += 2)
        if (rlens[i - 2] == rlens[i + 2]
          && rlens[i + 2] == rlens[i - 1]
          && rlens[i - 1] == rlens[i + 1]
          && rlens[i - 1] * 3 == rlens[i]
          // white around the black pattern?  Not part of spec
          && (rlens[i - 3] == 0 // beginning
            || i + 3 > length   // end
            || rlens[i - 3] * 3 >= rlens[i] * 4 || rlens[i + 3] * 3 >= rlens[i] * 4)
          )
            runsbad += N3;
    return runsbad;
}

static int badcheck()
{
    unsigned char x, y, h, b, b1;
    unsigned thisbad = 0;
    int bw = 0;

    // blocks of same color.
    for (y = 0; y < WD - 1; y++)
        for (x = 0; x < WD - 1; x++)
            if ((QRBIT(x, y) && QRBIT(x + 1, y) && QRBIT(x, y + 1) && QRBIT(x + 1, y + 1))      // all black
              || !(QRBIT(x, y) || QRBIT(x + 1, y) || QRBIT(x, y + 1) || QRBIT(x + 1, y + 1)))   // all white
                thisbad += N2;

    // X runs
    for (y = 0; y < WD; y++) {
        rlens[0] = 0;
        for (h = b = x = 0; x < WD; x++) {
            if ((b1 = QRBIT(x, y)) == b)
                rlens[h]++;
            else
                rlens[++h] = 1;
            b = b1;
            bw += b ? 1 : -1;
        }
        thisbad += badruns(h);
    }

    // black/white imbalance
    if (bw < 0)
        bw = -bw;

    unsigned long big = bw;
    unsigned count = 0;
    big += big << 2;
    big <<= 1;
    while (big > WD * WD)
        big -= WD * WD, count++;
    thisbad += count * N4;

    // Y runs
    for (x = 0; x < WD; x++) {
        rlens[0] = 0;
        for (h = b = y = 0; y < WD; y++) {
            if ((b1 = QRBIT(x, y)) == b)
                rlens[h]++;
            else
                rlens[++h] = 1;
            b = b1;
        }
        thisbad += badruns(h);
    }
    return thisbad;
}

// final format bits with mask
// level << 3 | mask
static const unsigned fmtword[] PROGMEM = {
    0x77c4, 0x72f3, 0x7daa, 0x789d, 0x662f, 0x6318, 0x6c41, 0x6976,     //L
    0x5412, 0x5125, 0x5e7c, 0x5b4b, 0x45f9, 0x40ce, 0x4f97, 0x4aa0,     //M
    0x355f, 0x3068, 0x3f31, 0x3a06, 0x24b4, 0x2183, 0x2eda, 0x2bed,     //Q
    0x1689, 0x13be, 0x1ce7, 0x19d0, 0x0762, 0x0255, 0x0d0c, 0x083b,     //H
};

static void addfmt(unsigned char masknum)
{
    unsigned fmtbits;
    unsigned char i, lvl = ECCLEVEL - 1;

    fmtbits = pgm_read_word(&fmtword[masknum + (lvl << 3)]);
    // low byte
    for (i = 0; i < 8; i++, fmtbits >>= 1)
        if (fmtbits & 1) {
            SETQRBIT(WD - 1 - i, 8);
            if (i < 6)
                SETQRBIT(8, i);
            else
                SETQRBIT(8, i + 1);
        }
    // high byte
    for (i = 0; i < 7; i++, fmtbits >>= 1)
        if (fmtbits & 1) {
            SETQRBIT(8, WD - 7 + i);
            if (i)
                SETQRBIT(6 - i, 8);
            else
                SETQRBIT(7, 8);
        }
}

void qrencode()
{
    unsigned mindem = 30000;
    unsigned char best = 0;
    unsigned char i;
    unsigned badness;

    stringtoqr();
    fillframe();                // Inisde loop to avoid having separate mask buffer
    memcpy(strinbuf, qrframe, WD * WDB);
    for (i = 0; i < 8; i++) {
        applymask(i);           // returns black-white imbalance
        badness = badcheck();
#if 0                           //ndef PUREBAD
        if (badness < WD * WD * 5 / 4) {        // good enough - masks grow in compute complexity
            best = i;
            break;
        }
#endif
        if (badness < mindem) {
            mindem = badness;
            best = i;
        }
        if (best == 7)
            break;              // don't increment i to avoid redoing mask
        memcpy(qrframe, strinbuf, WD * WDB);    // reset filled frame
    }
    if (best != i)              // redo best mask - none good enough, last wasn't best
        applymask(best);
    addfmt(best);               // add in final format bytes
}
