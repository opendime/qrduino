#define WD (41)
#define WDB ((WD+7)/8)
#ifndef __AVR__
#define PROGMEM
#define memcpy_P memcpy
#define __LPM(x) *x
#else
#include <avr/pgmspace.h>
#endif
static const unsigned char framebase[] PROGMEM = {
0xfe,0x00,0x00,0x00,0x3f,0x80,
0x82,0x00,0x00,0x00,0x20,0x80,
0xba,0x00,0x00,0x00,0x2e,0x80,
0xba,0x00,0x00,0x00,0x2e,0x80,
0xba,0x00,0x00,0x00,0x2e,0x80,
0x82,0x00,0x00,0x00,0x20,0x80,
0xfe,0xaa,0xaa,0xaa,0xbf,0x80,
0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0xf8,0x00,
0x00,0x80,0x00,0x00,0x88,0x00,
0xfe,0x00,0x00,0x00,0xa8,0x00,
0x82,0x00,0x00,0x00,0x88,0x00,
0xba,0x00,0x00,0x00,0xf8,0x00,
0xba,0x00,0x00,0x00,0x00,0x00,
0xba,0x00,0x00,0x00,0x00,0x00,
0x82,0x00,0x00,0x00,0x00,0x00,
0xfe,0x00,0x00,0x00,0x00,0x00,
};

static const unsigned char framask[] PROGMEM = {
0xff,0x80,0x00,0x00,0x7f,0x80,
0xff,0x80,0x00,0x00,0x7f,0x80,
0xff,0x80,0x00,0x00,0x7f,0x80,
0xff,0x80,0x00,0x00,0x7f,0x80,
0xff,0x80,0x00,0x00,0x7f,0x80,
0xff,0x80,0x00,0x00,0x7f,0x80,
0xff,0xff,0xff,0xff,0xff,0x80,
0xff,0x80,0x00,0x00,0x7f,0x80,
0xff,0x80,0x00,0x00,0x7f,0x80,
0x02,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0xf8,0x00,
0xff,0x80,0x00,0x00,0xf8,0x00,
0xff,0x80,0x00,0x00,0xf8,0x00,
0xff,0x80,0x00,0x00,0xf8,0x00,
0xff,0x80,0x00,0x00,0xf8,0x00,
0xff,0x80,0x00,0x00,0x00,0x00,
0xff,0x80,0x00,0x00,0x00,0x00,
0xff,0x80,0x00,0x00,0x00,0x00,
0xff,0x80,0x00,0x00,0x00,0x00,
};